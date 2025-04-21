#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Uso: ./consumer <topico> <cola>\n";
        return 1;
    }

    std::string topicName = argv[1];
    std::string queueName = argv[2];
    std::string command = "CONSUME:" + topicName + ":" + queueName;

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

    send(sock, command.c_str(), command.length(), 0);

    char buffer[4096] = {0};
    read(sock, buffer, 4096);
    
    if (std::string(buffer) == "NO_MESSAGE") {
        std::cout << "No hay mensajes disponibles en la cola" << std::endl;
    } else {
        std::cout << "Mensaje recibido: " << buffer << std::endl;
    }

    close(sock);
    return 0;
}
