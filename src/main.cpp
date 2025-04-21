#include "../include/broker.h"
#include <csignal>

Broker* globalBroker = nullptr;

void signalHandler(int signal) {
    if (globalBroker) {
        std::cout << "Cerrando broker..." << std::endl;
        globalBroker->stop();
    }
}

void printHelp() {
    std::cout << "Uso: ./broker <puerto>" << std::endl;
    std::cout << "Puerto por defecto: " << DEFAULT_PORT << std::endl;
    std::cout << "\nComandos del cliente:" << std::endl;
    std::cout << "  CREATE_TOPIC:<nombre>                - Crea un tópico" << std::endl;
    std::cout << "  DELETE_TOPIC:<nombre>                - Elimina un tópico" << std::endl;
    std::cout << "  LIST_TOPICS                          - Lista todos los tópicos" << std::endl;
    std::cout << "  CREATE_QUEUE:<topico>:<nombre>       - Crea una cola en un tópico" << std::endl;
    std::cout << "  DELETE_QUEUE:<topico>:<nombre>       - Elimina una cola" << std::endl;
    std::cout << "  LIST_QUEUES:<topico>                 - Lista todas las colas en un tópico" << std::endl;
    std::cout << "  PRODUCE:<topico>:<mensaje>           - Publica un mensaje en un tópico" << std::endl;
    std::cout << "  CONSUME:<topico>:<cola>              - Consume un mensaje de una cola" << std::endl;
}

int main(int argc, char *argv[]) {
    int port = DEFAULT_PORT;
    
    if (argc > 1) {
        if (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h") {
            printHelp();
            return 0;
        }
        
        try {
            port = std::stoi(argv[1]);
        } catch (const std::exception& e) {
            std::cerr << "Puerto inválido. Usando puerto por defecto: " << DEFAULT_PORT << std::endl;
            port = DEFAULT_PORT;
        }
    }
    
    // Configurar manejador de señales
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    Broker broker(port);
    globalBroker = &broker;
    
    // Iniciar el broker
    std::cout << "Iniciando broker en puerto " << port << std::endl;
    std::cout << "Presiona Ctrl+C para detener" << std::endl;
    
    broker.start();
    
    return 0;
}