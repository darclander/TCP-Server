#include <iostream>
#include <WS2tcpip.h>
#include <winsock2.h>


#pragma comment (lib, "ws2_32.lib")

// This method is for some reason not defined in ws2tcpip.h
const char *inet_ntop(int af, const void *src, char *dst, socklen_t size) {
  struct sockaddr_storage ss;
  unsigned long s = size;

  ZeroMemory(&ss, sizeof(ss));
  ss.ss_family = af;

  switch(af) {
    case AF_INET:
      ((struct sockaddr_in *)&ss)->sin_addr = *(struct in_addr *)src;
      break;
    case AF_INET6:
      ((struct sockaddr_in6 *)&ss)->sin6_addr = *(struct in6_addr *)src;
      break;
    default:
      return NULL;
  }
  /* cannot direclty use &size because of strict aliasing rules */
  return (WSAAddressToString((struct sockaddr *)&ss, sizeof(ss), NULL, dst, &s) == 0)?
          dst : NULL;
} 

int main() {

    // Initialize winsock
    WSADATA wsData;
    WORD ver = MAKEWORD(2, 2);
    int wsOk = WSAStartup(ver, &wsData);

    if (wsOk != 0) {
        std::cerr << "Can't initialize winsock! Quitting" << std::endl;
        return -1;
    }

    // Create a socket
    SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
    if (listening == INVALID_SOCKET) {
        std::cerr << "Can't create socket! Quitting" << std::endl;
        return -1;
    }

    // Bind the ip address and port to a socket
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(54000);
    hint.sin_addr.S_un.S_addr = INADDR_ANY; // Could also use inet_pton ...

    bind(listening, (sockaddr*)&hint, sizeof(hint));

    // Tell winsock the socket is for listening
    listen(listening, SOMAXCONN); // SOMAXCONN - 0x7fffffff

    // Wait for a connection
    sockaddr_in client;
    int clientSize = sizeof(client);

    SOCKET clientSocket = accept(listening, (sockaddr*)&client, &clientSize);
    if  (clientSocket == INVALID_SOCKET) {
        // ERROR
    }

    char host[NI_MAXHOST]; // Client's remote name
    char service[NI_MAXSERV]; // Service (i.e port) the client is connected on

    ZeroMemory(host, NI_MAXHOST); // Same as memset(host, 0, NI_MAXHOST);
    ZeroMemory(service, NI_MAXSERV);

    if(getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0) {
        std::cout << host << " connected on port " << service << std::endl;
    } else {
        inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
        std::cout << host << " connected on port " << ntohs(client.sin_port) << std::endl;
    }

    // Close listening socket
    closesocket(listening);

    // While loop: accept and echo message back to client
    char buf[4096];

    while( true ) {
        ZeroMemory(buf, 4096);
        
        // Wait for client to send data
        int bytesReceived = recv(clientSocket, buf, 4096, 0);

        if(bytesReceived == SOCKET_ERROR) {
            std::cerr << "Error in recv(). Quitting!" << std::endl;
            break;
        }

        if (bytesReceived == 0) {
            std::cout << "Client disconnected" << std::endl;
            break;
        }

        // Echo message back to client
        int s = send(clientSocket, buf, bytesReceived + 1, 0);
        std::cout << s << std::endl;

    }

    // Close the socket
    closesocket(clientSocket);

    // Cleanup winsock
    WSACleanup();

    return 0;
}