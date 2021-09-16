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
#include <thread>
#include <unordered_map>
#include "uv_utils.h"
#include "connection.h"
#include "worker.h"

namespace sw::vengine {

struct ReactorOptions {
    TcpOptions tcp_opts;

    ConnectionOptions connection_opts;
};

struct ReplyContext {
    explicit ReplyContext(Reply r) : reply(std::move(r)) {
        buf.base = reply.reply.data();
        buf.len = reply.reply.size();
    }

    Reply reply;

    uv_buf_t buf;
};

class Reactor {
public:
    Reactor(const ReactorOptions &opts, const WorkerPoolSPtr &worker_pool);

    Reactor(const Reactor &) = delete;
    Reactor& operator=(const Reactor &) = delete;

    Reactor(Reactor &&) = delete;
    Reactor& operator=(Reactor &&) = delete;

    ~Reactor();

    void send(Reply reply);

private:
    static void _on_connect(uv_stream_t *server, int status);

    static void _on_close(uv_handle_t *handle);

    static void _on_alloc(uv_handle_t *handle, std::size_t suggested_size, uv_buf_t *buf);

    static void _on_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf);

    static void _on_stop(uv_async_t *handle);

    static void _on_reply(uv_async_t *handle);

    static void _on_write(uv_write_t *req, int status);

    static void _on_timer(uv_timer_t *handle);

    void _stop();

    void _notify();

    std::pair<uint64_t, TcpUPtr> _create_client();

    void _close_client(uv_handle_t *handle);

    uint64_t _connection_id() {
        return _connection_cnt++;
    }

    void _send();

    void _send(Reply reply);

    void _dispatch(Connection &connection, std::vector<RespCommand> requests);

    ReactorOptions _opts;

    LoopUPtr _loop;

    TcpUPtr _server;

    AsyncUPtr _stop_async;

    AsyncUPtr _reply_async;

    std::thread _loop_thread;

    uint64_t _connection_cnt = 0;

    std::unordered_map<uint64_t, uv_tcp_t*> _connections;

    std::mutex _mutex;

    std::vector<Reply> _replies;

    WorkerPoolSPtr _worker_pool;

    uv_timer_t timer;
};

}

#endif // end SW_VECTOR_ENGINE_REACTOR_H
