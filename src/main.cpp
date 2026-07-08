#include <iostream>
#include "server/TcpServer.hpp"
#include "eviction/LRUPolicy.hpp"
#include "store/StoreEngine.hpp"

int main() {
    try {
        auto& store = StoreEngine::getInstance();
        store.setEvictionPolicy(std::make_unique<LRUPolicy>());

        TcpServer server("0.0.0.0", 6379);
        server.start();
    } catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}