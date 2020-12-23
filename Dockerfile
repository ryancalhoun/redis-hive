FROM ubuntu:18.04

RUN apt-get update
RUN apt-get install -y netcat

WORKDIR app
COPY bin/redis-hive .

ENTRYPOINT ["./redis-hive"]
