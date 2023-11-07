
#pragma once
#include <iostream>
#include <memory>
#include <string>

#include "MyHttpServer.h"
#include "RemoteCall.pb.h"
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_format.h"

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>


#include "RemoteCall.grpc.pb.h"
#include "Cache.h"


using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using SDCS::RemoteCall;
using SDCS::StoreRequest;
using SDCS::GetandRemoveRequest;
using SDCS::Reply;


// Logic and data behind the server's behavior.
class SDCSServiceImpl final : public RemoteCall::Service {
public:
  SDCSServiceImpl(MyHttpServer& myhttpserver) : myhttpserver_(myhttpserver){
  }

  Status NotifyNext(ServerContext* context, const SDCS::EmptyMessage* request,
                  Reply* reply) override { 
    myhttpserver_.getBalancer().NextServer();
    return Status::OK;
  }

  Status Store(ServerContext* context, const StoreRequest* request,
                  Reply* reply) override { 
    std::string key = request->key();
    std::string value = request->value();
    myhttpserver_.getCache().InsertOrUpdate(key, value);
    return Status::OK;
  }
  
  Status Get(ServerContext* context, const GetandRemoveRequest* request,
                       Reply* reply) override {
    std::string key = request->key();
    std::string value;
    if(myhttpserver_.getCache().ContainsKey(key)){
      myhttpserver_.getCache().GetValue(key,value);
      std::string reply_str= "{\"" + key + "\":" + value + "}";
      reply->set_reply_message(reply_str);
    }
    else reply->set_reply_message("NULL");
    return Status::OK;
  }
  
   Status Remove(ServerContext* context, const GetandRemoveRequest* request,
                       Reply* reply) override {
   
    std::string key = request->key();
    std::string value;
    if(myhttpserver_.getCache().ContainsKey(key)){
      myhttpserver_.getCache().RemoveKey(key);
      std::string reply_str = std::to_string(1);
      reply->set_reply_message(reply_str);
    }
    else reply->set_reply_message("NULL");
    return Status::OK;
  }

  private:
  MyHttpServer& myhttpserver_;
};

void RunRpcServer(uint16_t port,MyHttpServer& myhttpserver) {
  std::string server_address = absl::StrFormat("0.0.0.0:%d", port);
  SDCSServiceImpl service(myhttpserver);

  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);
  // Finally assemble the server.
  std::unique_ptr<Server> server(builder.BuildAndStart());

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}


