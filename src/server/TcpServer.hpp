#pragma once
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string>
#include <functional>
#include <thread>
#include "protocol/RespParser.hpp"
#include "commands/ICommand.hpp"
#include "commands/CommandDispatcher.hpp"
#include "commands/SubscribeCommand.hpp"
#include "commands/UnsubscribeCommand.hpp"
#include "commands/PublishCommand.hpp"

class TcpServer {
private:
    int _socketFileDescriptor;
    int _port;
    RespParser _parser;
    CommandDispatcher _dispatcher;
    void handleClient(int clientFd);
public:
    TcpServer(const std::string& host, int port);
    void start();
};