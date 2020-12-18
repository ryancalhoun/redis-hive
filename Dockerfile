FROM ubuntu:18.04

WORKDIR app
COPY bin/redis-hive .

ENTRYPOINT ["./redis-hive"]
