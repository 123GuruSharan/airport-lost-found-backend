FROM gcc:13

WORKDIR /app

# Install standalone ASIO
RUN apt-get update && apt-get install -y libasio-dev

COPY . .

RUN g++ main.cpp -std=gnu++17 -pthread -O2 -o airport_server

CMD ["./airport_server"]
