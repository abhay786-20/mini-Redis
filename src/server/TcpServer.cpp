#include "server/TcpServer.hpp"
#include <iostream>
#include <stdexcept>
#include <arpa/inet.h>
#include <cstring>
#include "commands/SetCommand.hpp"
#include "commands/GetCommand.hpp"
#include "commands/DelCommand.hpp"
#include "commands/LPushCommand.hpp"
#include "commands/HSetCommand.hpp"

TcpServer::TcpServer(const std::string& host, int port) : _port(port), _socketFileDescriptor(-1) {
    // register all command factories on startup
    _dispatcher.registerCommand("SET", [](const std::vector<std::string>& tokens) {
    int64_t ttlMs = -1;
    if (tokens.size() >= 5 && tokens[3] == "PX") {
        ttlMs = std::stoll(tokens[4]);
    }
    return std::make_unique<SetCommand>(tokens[1], tokens[2], ttlMs);
});
    _dispatcher.registerCommand("GET", [](const std::vector<std::string>& tokens) {
        return std::make_unique<GetCommand>(tokens[1]);
    });
    _dispatcher.registerCommand("DEL", [](const std::vector<std::string>& tokens) {
        return std::make_unique<DelCommand>(tokens[1]);
    });
    _dispatcher.registerCommand("LPUSH", [](const std::vector<std::string>& tokens) {
        return std::make_unique<LPushCommand>(tokens[1], tokens[2]);
    });
    _dispatcher.registerCommand("HSET", [](const std::vector<std::string>& tokens) {
        return std::make_unique<HSetCommand>(tokens[1], tokens[2], tokens[3]);
    });
}

void TcpServer::start() {
    // 1. create socket
    // AF_INET = IPv4, SOCK_STREAM = TCP, 0 = default protocol
    _socketFileDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (_socketFileDescriptor == -1) {
        throw std::runtime_error("Failed to create socket");
    }

    // allow reuse of port immediately after server restart
    int opt = 1;
    setsockopt(_socketFileDescriptor, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 2. bind to port
    // sockaddr_in describes the address to bind to
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;           // IPv4
    serverAddr.sin_addr.s_addr = INADDR_ANY;   // bind to all interfaces (0.0.0.0)
    serverAddr.sin_port = htons(_port);        // port in network byte order

    if (bind(_socketFileDescriptor, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        throw std::runtime_error("Failed to bind to port " + std::to_string(_port));
    }

    // 3. listen — max 10 pending connections in queue
    if (::listen(_socketFileDescriptor, 10) == -1) {
        throw std::runtime_error("Failed to listen on socket");
    }

    std::cout << "Mini Redis listening on port " << _port << std::endl;

    // 4. accept loop — one client at a time
    while (true) {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);

        // blocks here until a client connects
        int clientFd = accept(_socketFileDescriptor, (struct sockaddr*)&clientAddr, &clientLen);
        if (clientFd == -1) {
            std::cerr << "Failed to accept client" << std::endl;
            continue;
        }

        std::cout << "Client connected" << std::endl;

        // handle client — read loop
        char buffer[4096];
        while (true) {
            memset(buffer, 0, sizeof(buffer));

            // read raw bytes from client
            int bytesRead = read(clientFd, buffer, sizeof(buffer) - 1);
            if (bytesRead <= 0) {
                // client disconnected
                std::cout << "Client disconnected" << std::endl;
                break;
            }

            std::string raw(buffer, bytesRead);

            try {
                // parse RESP → tokens → dispatch → get response
                auto tokens = _parser.parse(raw);
                std::string response = _dispatcher.dispatch(tokens);

                // send response back — RESP simple string format
                std::string resp = "+" + response + "\r\n";
                write(clientFd, resp.c_str(), resp.size());

            } catch (const std::exception& e) {
                // send error back to client in RESP error format
                std::string err = "-ERR " + std::string(e.what()) + "\r\n";
                write(clientFd, err.c_str(), err.size());
            }
        }

        close(clientFd);
    }

    close(_socketFileDescriptor);
}