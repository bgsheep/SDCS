#pragma once
#include<iostream>
#include <vector>
#include "Cache.h"
#include "httplib.h"
#include "nlohmann/json_fwd.hpp"
#include "RoundRobinBalancer.h"

class MyHttpServer {
    enum RPCType{GET,REMOVE,STORE};
public:

    MyHttpServer(const std::string& address, int port, Cache& cache,RoundRobinBalancer& balancer,int id);

    std::string StoreRemoteCall(const std::string& key,const std::string& value,const int&);

    std::string GetandRemoveCall(RPCType rpctype, const std::string& key);

    int ContainKeyCall(const std::string&);

    void NotifyNextCall();

    void Start();

    int getId(){
        return id_;
    }

    RoundRobinBalancer& getBalancer(){
        return balancer_;
    }

    Cache& getCache(){
        return cache_;
    }
private:
    RoundRobinBalancer& balancer_; 
    std::string address_;
    std::vector<std::string> serveraddress_;
    int port_;
    int id_;
    Cache& cache_;
    
};