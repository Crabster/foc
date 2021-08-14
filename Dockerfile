FROM ubuntu:20.04

ARG DEBIAN_FRONTEND=noninteractive

RUN apt update && \
    apt -y install build-essential \
                   cmake \
                   git \
                   antlr4 \
                   pkg-config \
                   uuid-dev \
                   nasm

COPY . /foc
RUN cd /foc && make && touch /main.foc

ENV PATH="/foc/build:${PATH}"
