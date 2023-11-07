

#include <iostream>
#include <memory>
#include <string>

#include "RemoteCall.pb.h"
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"

#include <grpcpp/grpcpp.h>


#include "RemoteCall.grpc.pb.h"


//ABSL_FLAG(std::string, target, "localhost:50051", "Server address");

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using SDCS::RemoteCall;
using SDCS::StoreRequest;
using SDCS::GetandRemoveRequest;
using SDCS::Reply;

class SDCSClient {
 public:
  SDCSClient(std::shared_ptr<Channel> channel)
      : stub_(RemoteCall::NewStub(channel)) {}

  // Assembles the client's payload, sends it and presents the response back
  // from the server.
  std::string NotifyNext() {
    ClientContext context;
    SDCS::EmptyMessage emptymessage;
    Reply reply;  
    // The actual RPC.
    Status status = stub_->NotifyNext(&context,emptymessage, &reply);

    // Act upon its status.
    if (status.ok()) {
      return reply.reply_message();
    } else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "RPC failed";
    }
  }

  std::string Store(const std::string& key,const std::string& value) {
    // Data we are sending to the server.
    StoreRequest request;

    // Container for the data we expect from the server.
    Reply reply;
    request.set_key(key);
    request.set_value(value);

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->Store(&context, request, &reply);

    // Act upon its status.
    if (status.ok()) {
      return reply.reply_message();
    } else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "RPC failed";
    }
  }
  
   std::string Get(const std::string& key) {
    // Follows the same pattern as SayHello.
    GetandRemoveRequest request;
    request.set_key(key);
    Reply reply;
    ClientContext context;

    // Here we can use the stub's newly available method we just added.
    Status status = stub_->Get(&context, request, &reply);
    if (status.ok()) {
      return reply.reply_message();
    } else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "RPC failed";
    }
  }
  
   std::string Remove(const std::string& key) {
    // Follows the same pattern as SayHello.
    GetandRemoveRequest request;
    request.set_key(key);
    Reply reply;
    ClientContext context;

    // Here we can use the stub's newly available method we just added.
    Status status = stub_->Remove(&context, request, &reply);
    if (status.ok()) {
      return reply.reply_message();
    } else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "RPC failed";
    }
  }


 private:
  std::unique_ptr<RemoteCall::Stub> stub_;
};

