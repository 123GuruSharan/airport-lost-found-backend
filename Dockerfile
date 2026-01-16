FROM gcc:13

WORKDIR /app

COPY . .

RUN g++ main.cpp -std=gnu++17 -pthread -O2 -o airport_server

CMD ["./airport_server"]
