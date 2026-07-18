#include "server/TcpServer.hpp"
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <arpa/inet.h>
#include <cstring>
#include "commands/SetCommand.hpp"
#include "commands/GetCommand.hpp"
#include "commands/DelCommand.hpp"
#include "commands/LPushCommand.hpp"
#include "commands/HSetCommand.hpp"
#include "commands/SubscribeCommand.hpp"
#include "commands/UnsubscribeCommand.hpp"
#include "commands/PublishCommand.hpp"
#include "commands/SaveCommand.hpp"
#include "commands/AuthCommand.hpp"
#include "persistence/SnapshotWriter.hpp"
#include "auth/AuthProxy.hpp"

TcpServer::TcpServer(const std::string& host, int port) : _port(port), _socketFileDescriptor(-1) {
    _dispatcher.registerCommand("SET", [](const std::vector<std::string>& tokens, int) {
        int64_t ttlMs = -1;
        if (tokens.size() >= 5 && tokens[3] == "PX") {
            ttlMs = std::stoll(tokens[4]);
        }
        return std::make_unique<SetCommand>(tokens[1], tokens[2], ttlMs);
    }, true);
    _dispatcher.registerCommand("GET", [](const std::vector<std::string>& tokens, int) {
        return std::make_unique<GetCommand>(tokens[1]);
    });
    _dispatcher.registerCommand("DEL", [](const std::vector<std::string>& tokens, int) {
        return std::make_unique<DelCommand>(tokens[1]);
    }, true);
    _dispatcher.registerCommand("LPUSH", [](const std::vector<std::string>& tokens, int) {
        return std::make_unique<LPushCommand>(tokens[1], tokens[2]);
    }, true);
    _dispatcher.registerCommand("HSET", [](const std::vector<std::string>& tokens, int) {
        return std::make_unique<HSetCommand>(tokens[1], tokens[2], tokens[3]);
    }, true);
    _dispatcher.registerCommand("SUBSCRIBE", [](const std::vector<std::string>& tokens, int clientFd) {
        return std::make_unique<SubscribeCommand>(tokens[1], clientFd);
    });
    _dispatcher.registerCommand("UNSUBSCRIBE", [](const std::vector<std::string>& tokens, int clientFd) {
        return std::make_unique<UnsubscribeCommand>(tokens[1], clientFd);
    });
    _dispatcher.registerCommand("PUBLISH", [](const std::vector<std::string>& tokens, int) {
        return std::make_unique<PublishCommand>(tokens[1], tokens[2]);
    });
    _dispatcher.registerCommand("SAVE", [](const std::vector<std::string>&, int) {
        return std::make_unique<SaveCommand>();
    });
    _dispatcher.registerCommand("AUTH", [](const std::vector<std::string>& tokens, int clientFd) {
        if (tokens.size() < 2) {
            throw std::invalid_argument("wrong number of arguments for 'AUTH' command");
        }
        return std::make_unique<AuthCommand>(tokens[1], clientFd);
    });
}

void TcpServer::start() {
    _socketFileDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (_socketFileDescriptor == -1) {
        throw std::runtime_error("Failed to create socket");
    }

    int opt = 1;
    setsockopt(_socketFileDescriptor, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(_port);

    if (bind(_socketFileDescriptor, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        throw std::runtime_error("Failed to bind to port " + std::to_string(_port));
    }

    if (::listen(_socketFileDescriptor, 10) == -1) {
        throw std::runtime_error("Failed to listen on socket");
    }

    std::cout << "Mini Redis listening on port " << _port << std::endl;

    while (true) {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);

        int clientFd = accept(_socketFileDescriptor, (struct sockaddr*)&clientAddr, &clientLen);
        if (clientFd == -1) {
            std::cerr << "Failed to accept client" << std::endl;
            continue;
        }

        std::cout << "Client connected: fd=" << clientFd << std::endl;

        std::thread([this, clientFd]() {
            handleClient(clientFd);
        }).detach();
    }

    close(_socketFileDescriptor);
}

void TcpServer::loadAof(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cout << "No AOF file found at " << path << ", starting with empty store" << std::endl;
        return;
    }

    std::cout << "Replaying AOF from " << path << "..." << std::endl;
    std::string line;
    int replayed = 0;
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        std::vector<std::string> tokens;
        std::istringstream iss(line);
        std::string token;
        while (iss >> token) {
            tokens.push_back(token);
        }
        if (tokens.empty()) continue;

        try {
            _dispatcher.dispatch(tokens, -1, false);
            replayed++;
        } catch (const std::exception& e) {
            std::cerr << "AOF replay error on line \"" << line << "\": " << e.what() << std::endl;
        }
    }
    std::cout << "AOF replay complete: " << replayed << " commands replayed" << std::endl;
}

bool TcpServer::loadSnapshot(const std::string& path) {
    return SnapshotWriter::load(path);
}

void TcpServer::handleClient(int clientFd) {
    char buffer[4096];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytesRead = read(clientFd, buffer, sizeof(buffer) - 1);
        if (bytesRead <= 0) {
            std::cout << "Client disconnected: fd=" << clientFd << std::endl;
            break;
        }

        std::string raw(buffer, bytesRead);

        try {
            auto tokens = _parser.parse(raw);
            std::string response = AuthProxy::getInstance().dispatch(_dispatcher, tokens, clientFd);
            write(clientFd, response.c_str(), response.size());
        } catch (const std::exception& e) {
            std::string err = "-ERR " + std::string(e.what()) + "\r\n";
            write(clientFd, err.c_str(), err.size());
        }
    }

    AuthProxy::getInstance().removeClient(clientFd);
    close(clientFd);
}