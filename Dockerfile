FROM ubuntu:18.04 AS build

RUN apt-get update && apt-get install -y g++ make

COPY Makefile .
COPY src src

RUN make

FROM ubuntu:18.04

COPY --from=build bin/redis-proxy .
