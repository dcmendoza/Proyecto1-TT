// broker.cpp
#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

std::queue<std::string> messageQueue;
std::mutex queueMutex;

void handleClient(int clientSocket)
{
  char buffer[1024] = {0};
  read(clientSocket, buffer, 1024);
  std::string request(buffer);

  if (request.rfind("PRODUCE:", 0) == 0)
  {
    std::string msg = request.substr(8);
    std::lock_guard<std::mutex> lock(queueMutex);
    messageQueue.push(msg);

    std::cout << "[BROKER] Mensaje recibido: " << msg << std::endl;

    std::string response = "ACK: Message received.\n";
    send(clientSocket, response.c_str(), response.size(), 0);
  }
  else if (request == "CONSUME")
  {
    std::string msg;
    {
      std::lock_guard<std::mutex> lock(queueMutex);
      if (!messageQueue.empty())
      {
        msg = messageQueue.front();
        messageQueue.pop();
      }
      else
      {
        msg = "NO_MESSAGE";
      }
    }
    send(clientSocket, msg.c_str(), msg.size(), 0);
  }
  else
  {
    std::string err = "INVALID COMMAND\n";
    send(clientSocket, err.c_str(), err.size(), 0);
  }

  close(clientSocket);
}

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    std::cerr << "Uso: ./broker <puerto>\n";
    return 1;
  }

  int PORT = std::stoi(argv[1]);
  int server_fd, new_socket;
  struct sockaddr_in address;
  int opt = 1;
  socklen_t addrlen = sizeof(address);

  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  bind(server_fd, (struct sockaddr *)&address, sizeof(address));
  listen(server_fd, 10);

  std::cout << "Broker en puerto " << PORT << " esperando conexiones...\n";

  while (true)
  {
    new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
    std::thread(handleClient, new_socket).detach();
  }

  return 0;
}
