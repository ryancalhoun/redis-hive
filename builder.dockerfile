FROM ubuntu:18.04
RUN apt-get update
RUN apt-get install -y build-essential zip unzip jq
RUN apt-get install -y curl
