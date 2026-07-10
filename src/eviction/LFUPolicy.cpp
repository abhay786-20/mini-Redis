#include "eviction/LFUPolicy.hpp"
#include <stdexcept>

LFUPolicy::LFUPolicy() : _minFreq(0) {}

void LFUPolicy:: onGet(const std::string& key) {
    if (_freq.find(key) == _freq.end()) return;

    int freq = _freq[key];
    _buckets[freq].erase(_iterators[key]);

    if (_buckets[freq].empty()) {
        _buckets.erase(freq);
        if (_minFreq == freq ) {
            _minFreq++;
        }
    }

    _freq[key]++;
    _buckets[_freq[key]].push_front(key);
    _iterators[key] = _buckets[_freq[key]].begin();
}


void LFUPolicy::onSet(const std::string& key) {
    if (_freq.find(key) != _freq.end()) {
        onGet(key);
        return;
    }

    _freq[key] = 1;
    _buckets[1].push_front(key);
    _iterators[key] = _buckets[1].begin();
    _minFreq = 1;
}

std::string LFUPolicy::evict() {
    if (_buckets.empty()) {
        throw std::invalid_argument("No keys in cache");
    }

    auto& keys = _buckets[_minFreq];
    std::string keyToEvict = keys.back();
    keys.pop_back();

    if (keys.empty()) {
        _buckets.erase(_minFreq);
    }

    _freq.erase(keyToEvict);
    _iterators.erase(keyToEvict);

    return keyToEvict;
}   