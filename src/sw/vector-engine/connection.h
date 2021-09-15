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

#include "read_buffer.h"

namespace sw::vengine {

class Reactor;

struct ConnectionOptions {
    std::size_t read_buf_min_size;
    std::size_t read_buf_max_size;
};

class Connection {
public:
    Connection(uint64_t id, const ConnectionOptions &opts, Reactor &reactor) :
        _id(id),
        _read_buf(opts.read_buf_min_size, opts.read_buf_max_size),
        _reactor(reactor) {}

    Connection(const Connection &) = delete;
    Connection& operator=(const Connection &) = delete;

    Connection(Connection &&) = delete;
    Connection& operator=(Connection &&) = delete;

    ~Connection() = default;

    ReadBuffer& read_buffer() noexcept {
        return _read_buf;
    }

    Reactor& reactor() noexcept {
        return _reactor;
    }

    uint64_t id() const noexcept {
        return _id;
    }

private:
    uint64_t _id;

    ReadBuffer _read_buf;

    Reactor &_reactor;
};

}

#endif // end SW_VECTOR_ENGINE_CONNECTION_H
