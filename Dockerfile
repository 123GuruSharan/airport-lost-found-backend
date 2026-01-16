FROM gcc:latest

WORKDIR /app

COPY . .

RUN g++ main.cpp -std=gnu++17 -pthread -IC:\Users\hp\boost_1_90_0 -lws2_32 -lmswsock -o airport_server

CMD ["./airport_server"]
