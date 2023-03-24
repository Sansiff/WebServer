#pragma once

#include "EventLoop.h"
#include "Socket.h"

class Server {
 private:
  EventLoop *loop;

 public:
  Server(EventLoop *);
  ~Server();
  void handleReadEvent(int);
  void newConnection(Socket *serv_sock);
};