cmake_minimum_required(VERSION 3.10)
project(messaging_system)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

# Biblioteca principal
add_library(messaging STATIC
    src/common.cpp
    src/queue.cpp
    src/topic.cpp
    src/broker.cpp
)

# Ejecutable del broker
add_executable(broker src/main.cpp)
target_link_libraries(broker messaging pthread)

# Ejecutables de los clientes
add_executable(producer clients/producer.cpp)
target_link_libraries(producer messaging pthread)

add_executable(consumer clients/consumer.cpp)
target_link_libraries(consumer messaging pthread)