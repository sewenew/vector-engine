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
#include <charconv>
#include "errors.h"

namespace sw::vengine {

Connection::Connection(const ConnectionOptions &opts, Reactor &reactor) :
    _buf(opts.read_buf_min_size, opts.read_buf_max_size),
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
        args.reserve(argc);
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

std::optional<std::size_t> RespRequestParser::_parse_num(char c, std::string_view &buffer) {
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

std::optional<std::string> RespRequestParser::_parse_argv(std::string_view &buffer) {
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

    auto *last = buffer.data() + buffer.size();
    if (*(last - 2) != '\r' || *(last - 1) != '\n') {
        throw Error("expect '\\r\\n'");
    }

    return std::string(buffer.data(), len);
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

    auto conn = std::make_unique<Connection>(reactor->_opts);
    auto client = uv::make_tcp_client(reactor->_loop.get(), conn.get());
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

    std::tie(buf->base, buf->len) = conn->buffer().alloc(suggested_size);
}

void Reactor::_on_read(uv_stream_t *client, ssize_t nread, const uv_buf_t * /*buf*/) {
    if (nread > 0) {
        auto *conn = uv::handle_get_data<Connection>(client);

        auto &buffer = conn->buffer();
        buffer.update(nread);

        try {
            RespRequestParser parser;
            auto [requests, len] = parer.parse(buffer.data());
            buffer.dealloc(len);

            auto &reactor = conn->reactor();
            reactor->_submit_requests(std::move(requests));
        } catch (const Error &e) {
            // TODO: do log and send error reply
            uv::handle_close(client, _on_close);
            return;
        }
    }

    if (nread < 0) {
        if (nread != UV_EOF) {
            // TODO: do log
            // TODO: maybe send some error info to client before closing.
            // TODO: if the buffer is full, we need to tell client that the request is too large.
            uv::handle_close(client, _on_close);
        }
    }
}

Reactor::Reactor() : _loop(uv::make_loop()) {
    TcpOptions tcp_opts;
    _server = std::make_tcp_server(*_loop, tcp_opts, _on_connect);
    uv::handle_set_data(_server.get(), this);

    _opts.read_buf_min_size = 64 * 1024;
    _opts.read_buf_max_size = 20 * 1024 * 1024;
}

}
