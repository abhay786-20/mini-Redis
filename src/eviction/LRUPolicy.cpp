#include "LRUPolicy.hpp"
#include <stdexcept>

void LRUPolicy::onGet(const std::string& key) {
    auto it = _map.find(key);
    if (it == _map.end()) return;
    _order.splice(_order.begin(), _order, it->second);
}

void LRUPolicy::onSet(const std::string& key) {
    auto it = _map.find(key);
    if (it == _map.end()) {
        _order.push_front(key);
        _map[key] = _order.begin();
    } else {
        _order.splice(_order.begin(), _order, it->second);
    }
}

std::string LRUPolicy::evict() {
    if (_order.empty()) {
        throw std::invalid_argument("No keys in cache");
    }
   auto key = _order.back();  // least recently used
_order.pop_back();         // remove from back
_map.erase(key);
return key;
}