FROM ubuntu:18.04
RUN apt-get update -y
RUN apt-get install -y ruby redis-server

ENV BUNDLE_SILENCE_ROOT_WARNING=1
ARG BUNDLE_VERSION=2.1.4

RUN gem install bundler -v $BUNDLE_VERSION
