#ifndef QUEUE_H
#define QUEUE_H

#include <string>
#include <queue>
#include <mutex>
#include "common.h"

class Queue
{
private:
    std::string name;
    std::string topicName;
    std::queue<std::string> messageQueue;
    mutable std::mutex queueMutex; // Añadir 'mutable' aquí
    bool active;

public:
    Queue(const std::string &queueName, const std::string &parentTopic);
    ~Queue();
    // Operaciones básicas
    void enqueueMessage(const std::string &message);
    std::string consumeMessage();
    bool isEmpty() const;
    // Getters/Setters
    std::string getName() const;
    std::string getTopicName() const;
    bool isActive() const;
    void setActive(bool status);
};
#endif