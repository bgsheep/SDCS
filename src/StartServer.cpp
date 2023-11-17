
#include "MyHttpServer.h"
#include <exception>
#include <string>
#include <vector>
#include "RemoteCall.pb.h"
#include "grpcpp/server.h"
#include "remotecall_server.cc"
#include "remotecall_client.cc"

int main(int argc, char *argv[]) {
    if(argc > 2){
       std::cout<<"Usage: SDCS port"<<std::endl;
       return 0;
    }
    int port = std::stoi(argv[1]);
    Cache cache;
    RoundRobinBalancer balancer(3);
    MyHttpServer server("0.0.0.0",port, cache,balancer,port - 8081);
   

    std::thread([&server]() { server.Start(); }).detach();
    std::thread([&server]() { RunRpcServer(50051 ,server); }).detach();

    std::cout << "服务器已启动! 监听" << port << std::endl;
    

  std::this_thread::sleep_for(std::chrono::hours(1));
    return 0;
}
