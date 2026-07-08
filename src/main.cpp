#include <iostream>
#include "server/TcpServer.hpp"

int main() {
    try {
        TcpServer server("0.0.0.0", 6379);
        server.start();
    } catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}