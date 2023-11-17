class RoundRobinBalancer {
private:
    int current_server_;
    int servers_size_;

public:
    RoundRobinBalancer(int servers_size)
        :current_server_(0),servers_size_(servers_size) {}

    void NextServer() {
        current_server_++;
        current_server_ %= servers_size_;
    }

    int GetCurServer() {
        return current_server_;
    }

};