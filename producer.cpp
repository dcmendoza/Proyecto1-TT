// producer.cpp
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main()
{
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  sockaddr_in serv_addr = {AF_INET, htons(8080), INADDR_ANY};

  connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

  std::string message;
  std::cout << "Enter message to send: ";
  std::getline(std::cin, message);

  std::string full = "PRODUCE:" + message;
  send(sock, full.c_str(), full.length(), 0);

  char buffer[1024] = {0};
  read(sock, buffer, 1024);
  std::cout << "Broker response: " << buffer << "\n";

  close(sock);
  return 0;
}
