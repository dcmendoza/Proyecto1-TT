# Proyecto1-TT
Middleware de mensajería asincrónica implementado en un clúster con particionamiento, replicación y tolerancia a fallos. Permite la comunicación entre aplicaciones a través de colas y tópicos usando APIs REST y gRPC.

```shell
g++ broker.cpp -o broker -pthread
g++ load_balancer.cpp -o load_balancer -pthread
g++ producer.cpp -o producer
g++ consumer.cpp -o consumer
```

```shell
./broker 8081
./broker 8082
./broker 8083
```

```shell
./producer
./consumer
```