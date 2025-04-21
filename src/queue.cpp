#include "../include/queue.h"
#include <iostream>
#include "../include/common.h"

Queue::Queue(const std::string& queueName, const std::string& parentTopic)
    : name(queueName), topicName(parentTopic), active(true) {
    std::cout << "Cola '" << name << "' creada en tópico '" << topicName << "'" << std::endl;
}

Queue::~Queue() {
    std::lock_guard<std::mutex> lock(queueMutex);
    while (!messageQueue.empty()) {
        messageQueue.pop();
    }
    std::cout << "Cola '" << name << "' eliminada" << std::endl;
}

void Queue::enqueueMessage(const std::string& message) {
    std::lock_guard<std::mutex> lock(queueMutex);
    if (active) {
        messageQueue.push(message);
        std::cout << "Mensaje encolado en '" << name << "': " << message << std::endl;
    }
}

std::string Queue::consumeMessage() {
    std::lock_guard<std::mutex> lock(queueMutex);
    if (!active || messageQueue.empty()) {
        return RESPONSE_NO_MESSAGE;
    }
    
    std::string message = messageQueue.front();
    messageQueue.pop();
    std::cout << "Mensaje consumido de '" << name << "': " << message << std::endl;
    return message;
}

bool Queue::isEmpty() const {
    std::lock_guard<std::mutex> lock(queueMutex);
    return messageQueue.empty();
}

std::string Queue::getName() const {
    return name;
}

std::string Queue::getTopicName() const {
    return topicName;
}

bool Queue::isActive() const {
    return active;
}

void Queue::setActive(bool status) {
    std::lock_guard<std::mutex> lock(queueMutex);
    active = status;
}