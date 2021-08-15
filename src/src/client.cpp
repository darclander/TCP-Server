#include <iostream>
#include <string>
#include <WS2tcpip.h>
    

int inet_pton(int af, const char *src, void *dst)
{
  struct sockaddr_storage ss;
  int size = sizeof(ss);
  char src_copy[INET6_ADDRSTRLEN+1];

  ZeroMemory(&ss, sizeof(ss));
  /* stupid non-const API */
  strncpy (src_copy, src, INET6_ADDRSTRLEN+1);
  src_copy[INET6_ADDRSTRLEN] = 0;

  if (WSAStringToAddress(src_copy, af, NULL, (struct sockaddr *)&ss, &size) == 0) {
    switch(af) {
      case AF_INET:
    *(struct in_addr *)dst = ((struct sockaddr_in *)&ss)->sin_addr;
    return 1;
      case AF_INET6:
    *(struct in6_addr *)dst = ((struct sockaddr_in6 *)&ss)->sin6_addr;
    return 1;
    }
  }
  return 0;
}

void foo(SOCKET sock) {
  char buf[4096];
    ZeroMemory(buf, 4096);
    int bytesReceived = recv(sock, buf, 4096, 0);
    if (bytesReceived > 0) {
      // Echo response to console
      std::cout << "SERVER> " << std::string(buf, 0, bytesReceived) << std::endl;
    }
}

int main() {
    std::string ipAddress = "127.0.0.1";
    int port = 54000;

    // Init WinSock

    WSAData data;
    WORD ver = MAKEWORD(2, 2);
    int wsResult = WSAStartup(ver, &data);
    if(wsResult != 0) {
        std::cerr << "Can't start winsock, Err#" << wsResult << std::endl;
    }

    // Create socket
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == INVALID_SOCKET) {
        std::cerr << "Can't create socket, Err #" << WSAGetLastError() << std::endl;
        WSACleanup();
        return -1;
    }

    // Fill in a hint structure
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);
    inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

    // Connect to server
    int connResult = connect(sock, (sockaddr*)&hint, sizeof(hint));
    if (connResult == SOCKET_ERROR) {
        std::cerr << "Can't connect to server, Err#" << WSAGetLastError() << std::endl;
        closesocket(sock);
        WSACleanup();
        return -1;
    }

    // Do-while loop to send and receive data
    char buf[4096];
    std::string userInput;

    do {

        // Prompt the user for some text
        std::cout << "> ";


        std::getline(std::cin, userInput);

        if (userInput.size() > 0) { // Make sure the user typed something
            // Send the text
            int sendResult = send(sock, userInput.c_str(), userInput.size() + 1, 0);
            if (sendResult != SOCKET_ERROR) {
              std::cout << "Message sent!" << std::endl;
            }
        }

    } while (userInput.size() > 0);

    // Gracefully close down everything
    closesocket(sock);
    WSACleanup();
    return 0;
}