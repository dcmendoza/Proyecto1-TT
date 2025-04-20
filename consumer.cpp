// consumer.cpp
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main()
{
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  sockaddr_in serv_addr = {AF_INET, htons(8080), INADDR_ANY};

  connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

  std::string command = "CONSUME";
  send(sock, command.c_str(), command.length(), 0);

  char buffer[1024] = {0};
  read(sock, buffer, 1024);
  std::cout << "Received message: " << buffer << "\n";

  close(sock);
  return 0;
}
