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

#include "reactor.h"
#include <cassert>
#include <iostream>
#include <charconv>
#include "errors.h"

namespace sw::vengine {

Connection::Connection(const ConnectionOptions &opts, Reactor &reactor) :
    _read_buf(opts.read_buf_min_size, opts.read_buf_max_size),
    _reactor(reactor) {}

auto RespRequestParser::parse(std::string_view buffer) const
    -> std::pair<std::vector<RespRequest>, std::size_t> {
    auto *first = buffer.data();
    std::vector<RespRequest> requests;
    std::size_t bytes_parsed = 0;

    while (true) {
        auto argc = _parse_argc(buffer);
        if (!argc) {
            // Incomplete request.
            break;
        }
        RespRequest req;
        auto &args = req.args;
        args.reserve(*argc);
        auto idx = 0U;
        for ( ; idx != *argc; ++idx) {
            auto argv = _parse_argv(buffer);
            if (!argv) {
                // Incomplete request.
                break;
            }
            args.push_back(std::move(*argv));
        }

        if (idx < *argc) {
            // Incomplete request.
            break;
        }

        requests.push_back(std::move(req));
        bytes_parsed = (buffer.data() - first);
    }

    return std::make_pair(std::move(requests), bytes_parsed);
}

std::optional<std::size_t> RespRequestParser::_parse_num(char c, std::string_view &buffer) const {
    if (buffer.empty()) {
        return std::nullopt;
    }

    if (buffer.front() != c) {
        throw Error("expect " + std::string(1, c));
    }

    buffer.remove_prefix(1);

    if (buffer.empty()) {
        return std::nullopt;
    }

    std::size_t argc = 0;
    auto *last = buffer.data() + buffer.size();
    auto [ptr, err] = std::from_chars(buffer.data(), last, argc);
    if (err != std::errc()) {
        throw Error("expect a positive integer");
    }

    if (ptr + 2 > last) {
        return std::nullopt;
    }

    if (*ptr != '\r' || *(ptr + 1) != '\n') {
        throw Error("expect '\\r\\n'");
    }

    buffer.remove_prefix(ptr + 2 - buffer.data());

    return argc;
}

std::optional<std::string> RespRequestParser::_parse_argv(std::string_view &buffer) const {
    // $n\r\nxxxxx\r\n
    auto num = _parse_num('$', buffer);
    if (!num) {
        // Incomplete request.
        return std::nullopt;
    }

    auto len = *num;

    if (buffer.size() < len + 2) {
        return std::nullopt;
    }

    auto *last = buffer.data() + len;
    if (*last != '\r' || *(last + 1) != '\n') {
        throw Error("expect '\\r\\n'");
    }

    std::string argv(buffer.data(), len);

    buffer.remove_prefix(len + 2);

    return argv;
}

void Reactor::_on_connect(uv_stream_t *server, int status) {
    if (status < 0) {
        // TODO: do log instead of throw exception
        //throw UvError(status, "failed to create new connection");
        return;
    }

    assert(server != nullptr);

    auto *reactor = uv::handle_get_data<Reactor>(server);

    assert(reactor != nullptr);

    auto conn = std::make_unique<Connection>(reactor->_opts.connection_opts, *reactor);
    auto client = uv::make_tcp_client(*(reactor->_loop), conn.get());
    auto *cli = client.get();
    if (uv_accept(server, uv::to_stream(cli)) == 0) {
        uv_read_start(uv::to_stream(cli), _on_alloc, _on_read);
    } else {
        uv::handle_close(cli, _on_close);
    }

    client.release();
    conn.release();
}

void Reactor::_on_close(uv_handle_t *handle) {
    auto *conn = uv::handle_get_data<Connection>(handle);
    delete conn;

    auto *client = reinterpret_cast<uv_tcp_t *>(handle);
    delete client;
}

void Reactor::_on_alloc(uv_handle_t *handle, std::size_t suggested_size, uv_buf_t *buf) {
    auto *conn = uv::handle_get_data<Connection>(handle);

    std::tie(buf->base, buf->len) = conn->read_buffer().alloc(suggested_size);
}

void Reactor::_on_read(uv_stream_t *client, ssize_t nread, const uv_buf_t * /*buf*/) {
    if (nread > 0) {
        auto *conn = uv::handle_get_data<Connection>(client);

        auto &buffer = conn->read_buffer();
        buffer.occupy(nread);

        try {
            RespRequestParser parser;
            auto [requests, len] = parser.parse(buffer.data());

            if (!requests.empty()) {
                assert(len > 0);

                buffer.dealloc(len);

                for (const auto &ele : requests) {
                    for (const auto &e : ele.args) {
                        std::cout << e << std::endl;
                    }
                }
                std::cout << "--------" << std::endl;
            }

            auto &reactor = conn->reactor();
        } catch (const Error &e) {
            // TODO: do log and send error reply
            std::cerr << e.what() << ", " << buffer.data() << std::endl;
            uv::handle_close(client, _on_close);
            return;
        }
    }

    if (nread < 0) {
        if (nread != UV_EOF) {
            // TODO: do log
            // TODO: maybe send some error info to client before closing.
            // TODO: if the buffer is full, we need to tell client that the request is too large.
            std::cerr << "nread < 0" << std::endl;
            uv::handle_close(client, _on_close);
        }
    }
}

void Reactor::_on_stop(uv_async_t *handle) {
    assert(handle != nullptr);

    auto *reactor = uv::handle_get_data<Reactor>(handle);
    assert(reactor != nullptr);

    // TODO: clean up

    uv_stop(reactor->_loop.get());
}

Reactor::Reactor(const ReactorOptions &opts) :
    _loop(uv::make_loop()),
    _opts(opts) {
    _server = uv::make_tcp_server(*_loop, _opts.tcp_opts, _on_connect);
    uv::handle_set_data(_server.get(), this);

    _stop_async = uv::make_async(*_loop, _on_stop, this);

    _loop_thread = std::thread([this]() { uv_run(this->_loop.get(), UV_RUN_DEFAULT); });
}

Reactor::~Reactor() {
    if (_loop_thread.joinable()) {
        _loop_thread.join();
    }
}

void Reactor::_stop() {
    assert(_stop_async);

    uv_async_send(_stop_async.get());
}

}
