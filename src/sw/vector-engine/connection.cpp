/**************************************************************************
   Copyright (c) 2022 sewenew

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

#include "connection.h"
#include <cassert>
#include <iostream>
#include "reactor.h"
#include "resp.h"

namespace sw::vengine {

void Connection::on_alloc(uv_handle_t *handle, std::size_t suggested_size, uv_buf_t *buf) {
    auto *conn = uv::get_data<Connection>(handle);

    assert(conn != nullptr);

    std::tie(buf->base, buf->len) = conn->_read_buf.alloc(suggested_size);
}

void Connection::on_close(uv_handle_t *handle) {
    assert(handle != nullptr);

    auto *connection = uv::get_data<Connection>(handle);
    assert(connection != nullptr);

    connection->_reactor.remove_connection(connection->id());

    delete connection;

    auto *client = reinterpret_cast<uv_tcp_t*>(handle);
    delete client;
}

void Connection::on_read(uv_stream_t *client, ssize_t nread, const uv_buf_t * /*buf*/) {
    if (nread > 0) {
        auto *conn = uv::get_data<Connection>(client);

        auto &buffer = conn->_read_buf;
        buffer.occupy(nread);

        try {
            RespCommandParser parser;
            auto [requests, len] = parser.parse(buffer.data());

            if (!requests.empty()) {
                assert(len > 0);

                buffer.dealloc(len);

                conn->_reactor.dispatch(conn->id(), std::move(requests));
            }
        } catch (const Error &e) {
            // TODO: do log and send error reply
            std::cerr << e.what() << ", " << buffer.data() << std::endl;
            uv::handle_close(client, on_close);
            return;
        }
    }

    if (nread < 0) {
        if (nread != UV_EOF) {
            // TODO: do log
            // TODO: maybe send some error info to client before closing.
            // TODO: if the buffer is full, we need to tell client that the request is too large.
        } // else client closed the connection
        uv::handle_close(client, on_close);
    }
}

Connection::Connection(ConnectionId id,
        const ConnectionOptions &opts,
        Reactor &reactor) :
    _id(id),
    _read_buf(opts.read_buf_min_size, opts.read_buf_max_size),
    _reactor(reactor) {
}

}
