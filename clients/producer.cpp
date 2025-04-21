#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <sstream>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Uso: ./producer <comando> [argumentos...]\n";
        std::cerr << "Comandos disponibles:\n";
        std::cerr << "  create-topic <nombre>\n";
        std::cerr << "  delete-topic <nombre>\n";
        std::cerr << "  list-topics\n";
        std::cerr << "  create-queue <topico> <nombre>\n";
        std::cerr << "  delete-queue <topico> <nombre>\n";
        std::cerr << "  list-queues <topico>\n";
        std::cerr << "  produce <topico> <mensaje>\n";
        return 1;
    }

    std::string command = argv[1];
    std::string fullCommand;

    if (command == "create-topic" && argc == 3) {
        fullCommand = "CREATE_TOPIC:" + std::string(argv[2]);
    } else if (command == "delete-topic" && argc == 3) {
        fullCommand = "DELETE_TOPIC:" + std::string(argv[2]);
    } else if (command == "list-topics" && argc == 2) {
        fullCommand = "LIST_TOPICS";
    } else if (command == "create-queue" && argc == 4) {
        fullCommand = "CREATE_QUEUE:" + std::string(argv[2]) + ":" + std::string(argv[3]);
    } else if (command == "delete-queue" && argc == 4) {
        fullCommand = "DELETE_QUEUE:" + std::string(argv[2]) + ":" + std::string(argv[3]);
    } else if (command == "list-queues" && argc == 3) {
        fullCommand = "LIST_QUEUES:" + std::string(argv[2]);
    } else if (command == "produce" && argc >= 4) {
        fullCommand = "PRODUCE:" + std::string(argv[2]) + ":";
        for (int i = 3; i < argc; i++) {
            fullCommand += std::string(argv[i]) + " ";
        }
        fullCommand = fullCommand.substr(0, fullCommand.size() - 1);
    } else {
        std::cerr << "Comando no válido o argumentos incorrectos\n";
        return 1;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Error al crear socket\n";
        return 1;
    }

    sockaddr_in serv_addr = {0};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cerr << "Dirección inválida\n";
        return 1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Conexión fallida\n";
        return 1;
    }

    send(sock, fullCommand.c_str(), fullCommand.length(), 0);

    char buffer[4096] = {0};
    read(sock, buffer, 4096);
    std::cout << "Respuesta: " << buffer << std::endl;

    close(sock);
    return 0;
}
