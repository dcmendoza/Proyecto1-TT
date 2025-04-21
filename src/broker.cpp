#include "../include/broker.h"

Broker::Broker(int port)
    : port(port), serverSocket(-1), running(false) {
    std::cout << "Broker iniciado en puerto " << port << std::endl;
}

Broker::~Broker() {
    stop();
    std::lock_guard<std::mutex> lock(brokerMutex);
    for (auto& pair : topics) {
        delete pair.second;
    }
    topics.clear();
    std::cout << "Broker detenido" << std::endl;
}

void Broker::start() {
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        std::cerr << "Error al crear socket" << std::endl;
        return;
    }

    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr *)&address, sizeof(address)) < 0) {
        std::cerr << "Error en bind" << std::endl;
        close(serverSocket);
        serverSocket = -1;
        return;
    }

    if (listen(serverSocket, MAX_CLIENTS) < 0) {
        std::cerr << "Error en listen" << std::endl;
        close(serverSocket);
        serverSocket = -1;
        return;
    }

    running = true;
    std::cout << "Broker escuchando en puerto " << port << std::endl;

    while (running) {
        int clientSocket = accept(serverSocket, (struct sockaddr *)&address, &addrlen);
        if (clientSocket < 0) {
            if (running) {
                std::cerr << "Error en accept" << std::endl;
            }
            continue;
        }

        std::thread(&Broker::handleClient, this, clientSocket).detach();
    }
}

void Broker::stop() {
    running = false;
    if (serverSocket != -1) {
        close(serverSocket);
        serverSocket = -1;
    }
}

bool Broker::createTopic(const std::string& topicName) {
    if (topicName.empty()) {
        std::cerr << "No se puede crear un tópico con nombre vacío" << std::endl;
        return false;
    }

    std::lock_guard<std::mutex> lock(brokerMutex);
    if (topics.find(topicName) != topics.end()) {
        std::cout << "Tópico '" << topicName << "' ya existe" << std::endl;
        return false;
    }
    
    Topic* topic = new Topic(topicName);
    topics[topicName] = topic;
    return true;
}

bool Broker::deleteTopic(const std::string& topicName) {
    std::lock_guard<std::mutex> lock(brokerMutex);
    auto it = topics.find(topicName);
    if (it == topics.end()) {
        return false;
    }
    
    delete it->second;
    topics.erase(it);
    return true;
}

std::vector<std::string> Broker::listTopics() const {
    std::lock_guard<std::mutex> lock(brokerMutex);
    std::vector<std::string> topicNames;
    for (const auto& pair : topics) {
        topicNames.push_back(pair.first);
    }
    return topicNames;
}

bool Broker::createQueue(const std::string& topicName, const std::string& queueName) {
    if (topicName.empty() || queueName.empty()) {
        std::cerr << "No se puede crear una cola con nombre vacío o en un tópico con nombre vacío" << std::endl;
        return false;
    }

    std::lock_guard<std::mutex> lock(brokerMutex);
    auto it = topics.find(topicName);
    if (it == topics.end()) {
        return false;
    }
    
    return it->second->createQueue(queueName) != nullptr;
}

bool Broker::deleteQueue(const std::string& topicName, const std::string& queueName) {
    std::lock_guard<std::mutex> lock(brokerMutex);
    auto it = topics.find(topicName);
    if (it == topics.end()) {
        return false;
    }
    
    return it->second->deleteQueue(queueName);
}

std::vector<std::string> Broker::listQueues(const std::string& topicName) const {
    std::lock_guard<std::mutex> lock(brokerMutex);
    auto it = topics.find(topicName);
    if (it == topics.end()) {
        return std::vector<std::string>();
    }
    
    return it->second->listQueues();
}

bool Broker::publishMessage(const std::string& topicName, const std::string& message) {
    std::lock_guard<std::mutex> lock(brokerMutex);
    auto it = topics.find(topicName);
    if (it == topics.end()) {
        return false;
    }
    
    it->second->publishMessage(message);
    return true;
}

std::string Broker::consumeMessage(const std::string& topicName, const std::string& queueName) {
    std::lock_guard<std::mutex> lock(brokerMutex);
    auto it = topics.find(topicName);
    if (it == topics.end()) {
        return RESPONSE_ERROR + ": Tópico no encontrado";
    }
    
    return it->second->consumeMessage(queueName);
}

void Broker::handleClient(int clientSocket) {
    char buffer[MAX_BUFFER_SIZE] = {0};
    ssize_t bytesRead = read(clientSocket, buffer, MAX_BUFFER_SIZE - 1); // Asegurar espacio para null terminator
    
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0'; // Asegurar terminación del string
        std::string request(buffer);
        std::string response = parseAndExecuteCommand(request);
        send(clientSocket, response.c_str(), response.size(), 0);
    }
    
    close(clientSocket);
}

std::string Broker::parseAndExecuteCommand(const std::string& request) {
    std::cout << "Comando recibido: " << request << std::endl;
    
    // Crear tópico
    if (request.rfind(CMD_CREATE_TOPIC, 0) == 0) {
        std::string topicName = request.substr(CMD_CREATE_TOPIC.size());
        return createTopic(topicName) ? RESPONSE_OK + ": Tópico creado" : RESPONSE_ERROR + ": No se pudo crear el tópico";
    }
    
    // Eliminar tópico
    if (request.rfind(CMD_DELETE_TOPIC, 0) == 0) {
        std::string topicName = request.substr(CMD_DELETE_TOPIC.size());
        return deleteTopic(topicName) ? RESPONSE_OK + ": Tópico eliminado" : RESPONSE_ERROR + ": Tópico no encontrado";
    }
    
    // Listar tópicos
    if (request == CMD_LIST_TOPICS) {
        auto topics = listTopics();
        std::string response = RESPONSE_OK + ": ";
        for (const auto& topic : topics) {
            response += topic + ",";
        }
        return topics.empty() ? response + "Sin tópicos" : response.substr(0, response.size() - 1);
    }
    
    // Crear cola
    if (request.rfind(CMD_CREATE_QUEUE, 0) == 0) {
        std::string params = request.substr(CMD_CREATE_QUEUE.size());
        auto parts = splitString(params, ':');
        if (parts.size() != 2) {
            return RESPONSE_ERROR + ": Formato incorrecto. Uso: CREATE_QUEUE:topico:cola";
        }
        return createQueue(parts[0], parts[1]) ? RESPONSE_OK + ": Cola creada" : RESPONSE_ERROR + ": No se pudo crear la cola";
    }
    
    // Eliminar cola
    if (request.rfind(CMD_DELETE_QUEUE, 0) == 0) {
        std::string params = request.substr(CMD_DELETE_QUEUE.size());
        auto parts = splitString(params, ':');
        if (parts.size() != 2) {
            return RESPONSE_ERROR + ": Formato incorrecto. Uso: DELETE_QUEUE:topico:cola";
        }
        return deleteQueue(parts[0], parts[1]) ? RESPONSE_OK + ": Cola eliminada" : RESPONSE_ERROR + ": Cola no encontrada";
    }
    
    // Listar colas
    if (request.rfind(CMD_LIST_QUEUES, 0) == 0) {
        std::string topicName = request.substr(CMD_LIST_QUEUES.size());
        auto queues = listQueues(topicName);
        std::string response = RESPONSE_OK + ": ";
        for (const auto& queue : queues) {
            response += queue + ",";
        }
        return queues.empty() ? response + "Sin colas" : response.substr(0, response.size() - 1);
    }
    
    // Producir mensaje
    if (request.rfind(CMD_PRODUCE, 0) == 0) {
        std::string params = request.substr(CMD_PRODUCE.size());
        size_t pos = params.find(':');
        if (pos == std::string::npos) {
            return RESPONSE_ERROR + ": Formato incorrecto. Uso: PRODUCE:topico:mensaje";
        }
        std::string topicName = params.substr(0, pos);
        std::string message = params.substr(pos + 1);
        return publishMessage(topicName, message) ? RESPONSE_OK + ": Mensaje publicado" : RESPONSE_ERROR + ": Tópico no encontrado";
    }
    
    // Consumir mensaje
    if (request.rfind(CMD_CONSUME, 0) == 0) {
        std::string params = request.substr(CMD_CONSUME.size());
        auto parts = splitString(params, ':');
        if (parts.size() != 2) {
            return RESPONSE_ERROR + ": Formato incorrecto. Uso: CONSUME:topico:cola";
        }
        return consumeMessage(parts[0], parts[1]);
    }
    
    return RESPONSE_ERROR + ": Comando no reconocido";
}

bool Broker::hasTopic(const std::string& topicName) const {
    std::lock_guard<std::mutex> lock(brokerMutex);
    return topics.find(topicName) != topics.end();
}