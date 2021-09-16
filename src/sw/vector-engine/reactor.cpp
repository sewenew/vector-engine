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
#include "errors.h"
#include "resp.h"

namespace sw::vengine {

void Reactor::send(Reply reply) {
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _replies.push_back(std::move(reply));
    }

    _notify();
}

void Reactor::_on_connect(uv_stream_t *server, int status) {
    if (status < 0) {
        // TODO: do log instead of throw exception
        //throw UvError(status, "failed to create new connection");
        std::cout << "on connect error: " << uv::err_msg(status) << std::endl;
        return;
    }

    assert(server != nullptr);

    auto *reactor = uv::get_data<Reactor>(server);

    assert(reactor != nullptr);

    auto [id, client] = reactor->_create_client();
    std::cout << "in on connect: " << id << std::endl;
    auto *cli = client.get();
    if (uv_accept(server, uv::to_stream(cli)) == 0) {
        uv_read_start(uv::to_stream(cli), _on_alloc, _on_read);
    } else {
        std::cout << "refuse to accept" << std::endl;
        uv::handle_close(cli, _on_close);
    }

    reactor->_connections.emplace(id, client.release());
}

void Reactor::_on_close(uv_handle_t *handle) {
    assert(handle != nullptr);

    auto *connection = uv::get_data<Connection>(handle);
    assert(connection != nullptr);
    std::cout << "close conneciton: " << connection->id() << std::endl;

    connection->reactor()._close_client(handle);
}

void Reactor::_on_alloc(uv_handle_t *handle, std::size_t suggested_size, uv_buf_t *buf) {
    auto *conn = uv::get_data<Connection>(handle);

    std::tie(buf->base, buf->len) = conn->read_buffer().alloc(suggested_size);
}

void Reactor::_on_read(uv_stream_t *client, ssize_t nread, const uv_buf_t * /*buf*/) {
    if (nread > 0) {
        auto *conn = uv::get_data<Connection>(client);

        auto &buffer = conn->read_buffer();
        buffer.occupy(nread);

        try {
            RespCommandParser parser;
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

                conn->reactor()._dispatch(*conn, std::move(requests));
            }
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

    auto *reactor = uv::get_data<Reactor>(handle);
    assert(reactor != nullptr);

    // TODO: clean up

    uv_stop(reactor->_loop.get());
}

void Reactor::_on_reply(uv_async_t *handle) {
    assert(handle != nullptr);

    auto *reactor = uv::get_data<Reactor>(handle);
    assert(reactor != nullptr);

    reactor->_send();
}

void Reactor::_on_write(uv_write_t *req, int status) {
    if (status < 0) {
        // TODO: failed to do write.
        return;
    }

    auto *ctx = uv::get_data<ReplyContext>(req);
    delete ctx;

    delete req;
}

Reactor::Reactor(const ReactorOptions &opts, const WorkerPoolSPtr &worker_pool) :
    _loop(uv::make_loop()),
    _opts(opts),
    _worker_pool(worker_pool) {
    _server = uv::make_tcp_server(*_loop, _opts.tcp_opts, _on_connect, this);

    _stop_async = uv::make_async(*_loop, _on_stop, this);

    _reply_async = uv::make_async(*_loop, _on_reply, this);

    _loop_thread = std::thread([this]() { uv_run(this->_loop.get(), UV_RUN_DEFAULT); });
    uv_timer_init(_loop.get(), &timer);
    uv_timer_start(&timer, _on_timer, 2000, 2000);
}

void Reactor::_on_timer(uv_timer_t *handle) {
    std::cout << "timer" << std::endl;
}

Reactor::~Reactor() {
    _stop();

    if (_loop_thread.joinable()) {
        _loop_thread.join();
    }
}

void Reactor::_stop() {
    assert(_stop_async);

    uv_async_send(_stop_async.get());
}

void Reactor::_notify() {
    assert(_reply_async);

    uv_async_send(_reply_async.get());
}

std::pair<uint64_t, TcpUPtr> Reactor::_create_client() {
    auto id = _connection_id();
    auto conn = std::make_unique<Connection>(id, _opts.connection_opts, *this);

    auto client = uv::make_tcp_client(*_loop, conn.get());

    conn.release();

    return std::make_pair(id, std::move(client));
}

void Reactor::_close_client(uv_handle_t *handle) {
    assert(handle != nullptr);

    auto *connection = uv::get_data<Connection>(handle);
    assert(connection != nullptr);

    _connections.erase(connection->id());

    delete connection;

    auto *client = reinterpret_cast<uv_tcp_t*>(handle);
    delete client;
}

void Reactor::_send(Reply reply) {
    auto iter = _connections.find(reply.connection_id);
    if (iter == _connections.end()) {
        // Connection has been closed.
        return;
    }

    auto *client = iter->second;
    assert(client != nullptr);

    auto ctx = std::make_unique<ReplyContext>(std::move(reply));
    auto w = uv::make_write(*_loop, ctx.get());
    uv_write(w.get(), reinterpret_cast<uv_stream_t*>(client), &(ctx->buf), 1, _on_write);
    ctx.release();
    w.release();
}

void Reactor::_send() {
    std::vector<Reply> replies;
    {
        std::lock_guard<std::mutex> lock(_mutex);
        replies.swap(_replies);
    }

    for (auto &reply : replies) {
        _send(reply);
    }
}

void Reactor::_dispatch(Connection &connection, std::vector<RespCommand> requests) {
    auto id = connection.id();
    auto &worker = _worker_pool->fetch(id);
    Task task = {std::move(requests), id, this};
    worker.submit(std::move(task));
}

}
