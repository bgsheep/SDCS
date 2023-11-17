#include "Cache.h"
Cache::Cache() {
}

Cache::~Cache() {
    Clear();
}

void Cache::InsertOrUpdate(const std::string& key, const std::string& value) {
    cache_[key] = value;
}

bool Cache::GetValue(const std::string& key, std::string& value) {
    auto it = cache_.find(key);
    if (it != cache_.end()) {
        value = it->second;
        return true;
    }
    return false;
}

bool Cache::ContainsKey(const std::string& key) {
    return cache_.find(key) != cache_.end();
}

bool Cache::RemoveKey(const std::string& key) {
    return cache_.erase(key) > 0;
}

void Cache::Clear() {
    cache_.clear();
}