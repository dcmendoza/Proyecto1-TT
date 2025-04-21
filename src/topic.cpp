#include "../include/topic.h"

Topic::Topic(const std::string& topicName)
    : name(topicName), active(true) {
    std::cout << "Tópico '" << name << "' creado" << std::endl;
}

Topic::~Topic() {
    std::lock_guard<std::mutex> lock(topicMutex);
    for (auto& pair : queues) {
        delete pair.second;
    }
    queues.clear();
    std::cout << "Tópico '" << name << "' eliminado" << std::endl;
}

Queue* Topic::createQueue(const std::string& queueName) {
    std::lock_guard<std::mutex> lock(topicMutex);
    if (!active) {
        return nullptr;
    }
    
    if (queues.find(queueName) != queues.end()) {
        std::cout << "Cola '" << queueName << "' ya existe en tópico '" << name << "'" << std::endl;
        return nullptr;
    }
    
    Queue* queue = new Queue(queueName, name);
    queues[queueName] = queue;
    return queue;
}

bool Topic::deleteQueue(const std::string& queueName) {
    std::lock_guard<std::mutex> lock(topicMutex);
    if (!active) {
        return false;
    }
    
    auto it = queues.find(queueName);
    if (it == queues.end()) {
        return false;
    }
    
    delete it->second;
    queues.erase(it);
    return true;
}

std::vector<std::string> Topic::listQueues() const {
    std::lock_guard<std::mutex> lock(topicMutex);
    std::vector<std::string> queueNames;
    for (const auto& pair : queues) {
        queueNames.push_back(pair.first);
    }
    return queueNames;
}

void Topic::publishMessage(const std::string& message) {
    std::lock_guard<std::mutex> lock(topicMutex);
    if (!active || queues.empty()) {
        return;
    }
    
    for (auto& pair : queues) {
        pair.second->enqueueMessage(message);
    }
    std::cout << "Mensaje publicado en tópico '" << name << "'" << std::endl;
}

std::string Topic::consumeMessage(const std::string& queueName) {
    std::lock_guard<std::mutex> lock(topicMutex);
    if (!active) {
        return RESPONSE_ERROR + ": Tópico inactivo";
    }
    
    auto it = queues.find(queueName);
    if (it == queues.end()) {
        return RESPONSE_ERROR + ": Cola no encontrada";
    }
    
    return it->second->consumeMessage();
}

std::string Topic::getName() const {
    return name;
}

bool Topic::isActive() const {
    return active;
}

void Topic::setActive(bool status) {
    std::lock_guard<std::mutex> lock(topicMutex);
    active = status;
}

bool Topic::hasQueue(const std::string& queueName) const {
    std::lock_guard<std::mutex> lock(topicMutex);
    return queues.find(queueName) != queues.end();
}