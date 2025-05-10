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
#include "sw/vector-engine/connection.h"
#include "sw/vector-engine/task.h"
#include "sw/vector-engine/worker.h"
#include "sw/vector-engine/uv_utils.h"

namespace sw::vengine {

struct ReactorOptions {
    TcpOptions tcp_opts;

    ConnectionOptions connection_opts;

    ProtocolOptions protocol_opts;
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

    void send(std::vector<Reply> replies);

    void stop();

    void remove_connection(ConnectionId id) {
        _connections.erase(id);
    }

    void dispatch(ConnectionId id, std::vector<TaskUPtr> task, ResponseBuilderUPtr builder);

private:
    static void _on_connect(uv_stream_t *server, int status);

    static void _on_stop(uv_async_t *handle);

    static void _on_reply(uv_async_t *handle);

    static void _on_write(uv_write_t *req, int status);

    static void _on_timer(uv_timer_t *handle);

    void _notify();

    std::pair<ConnectionId, TcpUPtr> _create_client();

    ConnectionId _connection_id() {
        return _connection_cnt++;
    }

    void _send();

    void _send(Reply reply);

    ReactorOptions _opts;

    TcpUPtr _server;

    AsyncUPtr _stop_async;

    AsyncUPtr _reply_async;

    std::thread _loop_thread;

    uint64_t _connection_cnt = 0;

    std::unordered_map<ConnectionId, uv_tcp_t*> _connections;

    std::mutex _mutex;

    std::vector<Reply> _replies;

    WorkerPoolSPtr _worker_pool;

    uv_timer_t timer;

    LoopUPtr _loop;
};

}

#endif // end SW_VECTOR_ENGINE_REACTOR_H
