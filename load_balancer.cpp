// load_balancer.cpp
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

std::vector<int> brokerPorts = {8081, 8082, 8083};
int currentBroker = 0;
std::mutex brokerMutex;

int getNextBrokerPort()
{
  std::lock_guard<std::mutex> lock(brokerMutex);
  int port = brokerPorts[currentBroker];
  currentBroker = (currentBroker + 1) % brokerPorts.size();
  return port;
}

void forward(int clientSocket)
{
  char buffer[1024] = {0};
  int valread = read(clientSocket, buffer, 1024);
  if (valread <= 0)
  {
    close(clientSocket);
    return;
  }

  int brokerPort = getNextBrokerPort();

  int brokerSocket = socket(AF_INET, SOCK_STREAM, 0);
  sockaddr_in broker_addr;
  broker_addr.sin_family = AF_INET;
  broker_addr.sin_port = htons(brokerPort);
  broker_addr.sin_addr.s_addr = INADDR_ANY;

  if (connect(brokerSocket, (struct sockaddr *)&broker_addr, sizeof(broker_addr)) < 0)
  {
    std::cerr << "[BALANCER] Error al conectar con broker en puerto " << brokerPort << "\n";
    close(clientSocket);
    return;
  }

  send(brokerSocket, buffer, strlen(buffer), 0);

  char response[1024] = {0};
  int responseLen = read(brokerSocket, response, 1024);

  if (responseLen > 0)
  {
    send(clientSocket, response, responseLen, 0);
  }

  close(brokerSocket);
  close(clientSocket);
}

int main()
{
  int server_fd, new_socket;
  sockaddr_in address;
  int opt = 1;
  socklen_t addrlen = sizeof(address);

  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(8080); // El puerto que reemplaza al HAProxy

  bind(server_fd, (struct sockaddr *)&address, sizeof(address));
  listen(server_fd, 10);

  std::cout << "[BALANCER] Escuchando en puerto 8080...\n";

  while (true)
  {
    new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
    std::thread(forward, new_socket).detach();
  }

  return 0;
}
