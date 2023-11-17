#pragma once
#include <unordered_map>
#include <string>
class Cache {
public:
    Cache();
    ~Cache();

    void InsertOrUpdate(const std::string& key, const std::string& value);
    bool GetValue(const std::string& key, std::string& value);
    bool ContainsKey(const std::string& key);
    bool RemoveKey(const std::string& key);
    void Clear();

private:
    std::unordered_map<std::string, std::string> cache_;
};


