/**************************************************************************
   Copyright (c) 2021 sewenew

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
 *************************************************************************/

#ifndef SW_VECTOR_ENGINE_REACTOR_H
#define SW_VECTOR_ENGINE_REACTOR_H

#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <thread>
#include <optional>
#include "uv_utils.h"
#include "read_buffer.h"

namespace sw::vengine {

struct ConnectionOptions {
    std::size_t read_buf_min_size;
    std::size_t read_buf_max_size;
};

class Reactor;

class Connection {
public:
    Connection(const ConnectionOptions &opts, Reactor &reactor);

    Connection(const Connection &) = delete;
    Connection& operator=(const Connection &) = delete;

    Connection(Connection &&) = delete;
    Connection& operator=(Connection &&) = delete;

    ~Connection() = default;

    ReadBuffer& read_buffer() {
        return _read_buf;
    }

    Reactor& reactor() {
        return _reactor;
    }

private:
    ReadBuffer _read_buf;

    Reactor &_reactor;
};

struct RespRequest {
    std::vector<std::string> args;
};

class RespRequestParser {
public:
    // @return pair<vector<RespRequest>, number of bytes parsed>
    auto parse(std::string_view data) const
        -> std::pair<std::vector<RespRequest>, std::size_t>;

private:
    std::optional<std::size_t> _parse_num(char c, std::string_view &data) const;

    std::optional<std::size_t> _parse_argc(std::string_view &data) const {
        // *n\r\n
        return _parse_num('*', data);
    }

    std::optional<std::string> _parse_argv(std::string_view &data) const;
};

struct ReactorOptions {
    TcpOptions tcp_opts;

    ConnectionOptions connection_opts;
};

class Reactor {
public:
    explicit Reactor(const ReactorOptions &opts);

    Reactor(const Reactor &) = delete;
    Reactor& operator=(const Reactor &) = delete;

    Reactor(Reactor &&) = delete;
    Reactor& operator=(Reactor &&) = delete;

    ~Reactor();

private:
    static void _on_connect(uv_stream_t *server, int status);

    static void _on_close(uv_handle_t *handle);

    static void _on_alloc(uv_handle_t *handle, std::size_t suggested_size, uv_buf_t *buf);

    static void _on_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf);

    static void _on_stop(uv_async_t *handle);

    void _stop();

    ReactorOptions _opts;

    LoopUPtr _loop;

    TcpUPtr _server;

    AsyncUPtr _stop_async;

    std::thread _loop_thread;
};

}

#endif // end SW_VECTOR_ENGINE_REACTOR_H
