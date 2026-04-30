FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    g++ \
    make \
    curl \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

RUN g++ -std=c++17 -Wall -Wextra main.cpp -o mini-dfs

CMD ["./mini-dfs"]