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

#ifndef SW_VECTOR_ENGINE_CONNECTION_H
#define SW_VECTOR_ENGINE_CONNECTION_H

#include "sw/vector-engine/read_buffer.h"
#include "sw/vector-engine/protocol.h"
#include "sw/vector-engine/uv_utils.h"

namespace sw::vengine {

class Reactor;
class Task;

struct ConnectionOptions {
    std::size_t read_buf_min_size;
    std::size_t read_buf_max_size;
};

using ConnectionId = uint64_t;

class Connection {
public:
    Connection(ConnectionId id,
        const ConnectionOptions &opts,
        const ProtocolOptions &protocol_opts,
        Reactor &reactor);

    Connection(const Connection &) = delete;
    Connection& operator=(const Connection &) = delete;

    Connection(Connection &&) = delete;
    Connection& operator=(Connection &&) = delete;

    ~Connection() = default;

    ConnectionId id() const noexcept {
        return _id;
    }

    static void on_alloc(uv_handle_t *handle, std::size_t suggested_size, uv_buf_t *buf);

    static void on_close(uv_handle_t *handle);

    static void on_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf);

private:
    auto _parse_request(std::string_view buffer) const
        -> std::pair<std::vector<std::unique_ptr<Task>>, std::size_t>;

    ConnectionId _id;

    ReadBuffer _read_buf;

    ProtocolOptions _protocol_opts;

    Reactor &_reactor;
};

}

#endif // end SW_VECTOR_ENGINE_CONNECTION_H
