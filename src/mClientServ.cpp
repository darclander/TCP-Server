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

    fd_set master;
    FD_ZERO(&master);

    FD_SET(listening, &master);

    sockaddr addr;

    while (true) {
        fd_set copy = master;

        int socketCount = select(0, &copy, nullptr, nullptr, nullptr);

        for (int i = 0; i < socketCount; i++) {
            SOCKET sock = copy.fd_array[i];
            if (sock == listening) {
                // Accept a new connection
                SOCKET client = accept(listening, &addr, nullptr); // Look at second parameter for connecting client info.

                // Add the new connection to the list of connected clients
                FD_SET(client, &master);

                // Send a welcome message to the connected client
                std::string welcomeMsg = "Welcome to the awesome chat server!";
                char ipAddress[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &addr.sa_data, ipAddress, INET_ADDRSTRLEN);
                std::cout << ipAddress;
                send(client, welcomeMsg.c_str(), welcomeMsg.size() + 1, 0); // Look into for SDL

                // TODO: Broadcast we have a new connection

            } else {
                char buf[4096];
                ZeroMemory(buf, 4096);

                int bytesIn = recv(sock, buf, 4096, 0);
                if ( bytesIn <= 0) {
                    // Drop client
                    closesocket(sock);
                    FD_CLR(sock, &master);
                } else {
                    // Send message to the other clients
                    for (int i = 0; i < master.fd_count; i++) {
                        SOCKET outSock = master.fd_array[i];
                        if (outSock != listening && outSock != sock) {
                            send(outSock, buf, bytesIn, 0);
                        }
                    }
                }

                // Accept a new message
                // Send message to other clients and definiately NOT the listening socket
            }
        }

    }

    // Cleanup winsock
    WSACleanup();

    return 0;
}  

