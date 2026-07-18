#include <iostream>
#include "server/TcpServer.hpp"
#include "store/StoreEngine.hpp"
#include "eviction/LRUPolicy.hpp"
#include "auth/AuthProxy.hpp"

int main(int argc, char* argv[]) {
    try {
        auto& store = StoreEngine::getInstance();
        store.setEvictionPolicy(std::make_unique<LRUPolicy>());

        if (argc >= 2) {
            AuthProxy::getInstance().setPassword(argv[1]);
            std::cout << "Auth enabled: clients must AUTH before running other commands" << std::endl;
        }

        TcpServer server("0.0.0.0", 6379);
        if (!server.loadSnapshot()) {
            server.loadAof();
        }
        server.start();
    } catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}