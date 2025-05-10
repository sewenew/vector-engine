#include <iostream>
#include <chrono>
#include "sw/vector-engine/reactor.h"
#include "sw/vector-engine/worker.h"

int main() {
    try {
        auto pool = std::make_shared<sw::vengine::WorkerPool>(3);
        sw::vengine::ReactorOptions opts;
        opts.tcp_opts.ip = "127.0.0.1";
        opts.tcp_opts.port = 7777;
        opts.tcp_opts.backlog= 512;
        opts.tcp_opts.keepalive = std::chrono::seconds(30);
        opts.tcp_opts.nodelay = true;
        opts.connection_opts.read_buf_min_size = 64 * 1024;
        opts.connection_opts.read_buf_max_size = 20 * 1024 * 1024;
        opts.protocol_opts.type = sw::vengine::ProtocolType::RESP;
        sw::vengine::Reactor reactor(opts, pool);
        std::string input;
        while (std::getline(std::cin, input)) {
            if (input == "quit") {
                break;
            }
        }
        reactor.stop();
    } catch (const sw::vengine::Error &err) {
        std::cerr << err.what() << std::endl;
    }

    return 0;
}
