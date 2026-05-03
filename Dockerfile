FROM ubuntu:22.04

RUN for i in 1 2 3 4 5; do \
      apt-get update && break || sleep 10; \
    done \
    && apt-get install -y --no-install-recommends g++ cmake make curl \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN cmake -S . -B build && cmake --build build

WORKDIR /app/build
