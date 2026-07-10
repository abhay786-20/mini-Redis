#pragma once
#include <string>
#include <unordered_map>
#include <list>
#include "IEvictionPolicy.hpp"


class LFUPolicy : public IEvictionPolicy {
    private:
    std::unordered_map<std::string, int> _freq;
std::unordered_map<int, std::list<std::string>> _buckets;
std::unordered_map<std::string, std::list<std::string>::iterator> _iterators;
int _minFreq;
public:
    LFUPolicy();
    void onGet(const std::string& key) override;
    void onSet(const std::string& key) override;
    std::string evict() override;
};
