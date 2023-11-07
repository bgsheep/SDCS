#include "MyHttpServer.h"
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <string>

#include "RemoteCall.pb.h"
#include "remotecall_client.cc"
MyHttpServer::MyHttpServer(const std::string& address, int port, Cache& cache,RoundRobinBalancer& balancer,int id)
    : address_(address), port_(port), cache_(cache),balancer_(balancer),id_(id){
        serveraddress_={"172.16.238.10","172.16.238.11","172.16.238.12"};
    }


void MyHttpServer::Start() {
    httplib::Server server;

    server.Post("/", [this](const httplib::Request& req, httplib::Response& res) {
        if (req.has_header("Content-Type") && req.get_header_value("Content-Type") == "application/json") {
        nlohmann::json json_data;
        try {
            json_data = nlohmann::json::parse(req.body);
        } catch (const nlohmann::json::parse_error& e) {
            res.set_content("Failed to parse JSON data\n", "text/plain");
            return;
        }

         for (const auto& entry : json_data.items()) {
            std::string key = entry.key();
            std::string value = entry.value().dump();

            int containserverid = ContainKeyCall(key);
            if( containserverid != -1){
                StoreRemoteCall(key,value,containserverid);
            }
            else {
            if(balancer_.GetCurServer() == id_){
                cache_.InsertOrUpdate(key,value);
            }
            else {
                StoreRemoteCall(key,value,balancer_.GetCurServer());
            }
            NotifyNextCall();
            balancer_.NextServer();
            }
         }
            
        }
        else {
             res.set_content("Unknow header\n", "text/plain");
        }
             res.status = 200;
        
    });


     server.Get(R"(/(.*))", [this](const httplib::Request& req, httplib::Response& res) {
        std::string key = req.matches[1];
        std::string value;
        if(cache_.GetValue(key,value)){
            std::string response_str= "{\"" + key + "\":" + value + "}";
            res.set_content(response_str, "text/plain");
        }
        else {
            auto rpcreply = GetandRemoveCall(RPCType::GET,key);
            res.status = rpcreply == "" ? 404 : 200;
            res.set_content( rpcreply,"text/plain");
        }
        
        
    });

    server.Delete(R"(/(.*))", [this](const httplib::Request& req, httplib::Response& res) {
        std::string key = req.matches[1];
        std::string value;
        if(cache_.ContainsKey(key)){
            cache_.RemoveKey(key);
            res.set_content(std::to_string(1), "text/plain");
        }
        else {
            res.set_content(GetandRemoveCall(RPCType::REMOVE,key),"text/plain");
        };
        res.status = 200;
    });

   
    server.listen(address_.c_str(), port_);
}

auto MyHttpServer::StoreRemoteCall(const std::string& key,const std::string& value,const int& serverid) -> std::string{
      SDCSClient client(
      grpc::CreateChannel(serveraddress_[serverid]+ ":" + std::to_string(50051), grpc::InsecureChannelCredentials()));
      return client.Store(key,value);
}

auto MyHttpServer::ContainKeyCall(const std::string& key) -> int{
    std::string reply;
    for(auto serverid = 0;serverid < 3;serverid ++){
      SDCSClient client(
      grpc::CreateChannel(serveraddress_[serverid] + ":" + std::to_string(50051), grpc::InsecureChannelCredentials()));
     reply = client.Get(key);
      if(reply != "NULL"){
         return serverid;
      }
    }
    return -1;
}

auto MyHttpServer::GetandRemoveCall(RPCType rpctype,const std::string& key) -> std::string{
    std::string reply;
    for(auto serverid = (id_ + 1)%3;serverid != id_;serverid = (serverid + 1) % 3){
      SDCSClient client(
      grpc::CreateChannel(serveraddress_[serverid] + ":" + std::to_string(50051), grpc::InsecureChannelCredentials()));
      
      if(rpctype == RPCType::GET){
         reply = client.Get(key);
      }
      else {
         reply = client.Remove(key);
      }
      if(reply != "NULL"){
         return reply;
      }

    }

    if(rpctype == RPCType::GET){
         reply = "";
      }
    else {
         reply = '0';
      }
    return reply;
    
}


auto MyHttpServer::NotifyNextCall() -> void{
    for(auto serverid = (id_ + 1)%3;serverid != id_;serverid = (serverid + 1)%3){
      SDCSClient client(
      grpc::CreateChannel(serveraddress_[serverid]+ ":"+ std::to_string(50051), grpc::InsecureChannelCredentials()));
      client.NotifyNext();
    }
   
}
