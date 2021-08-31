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

#ifndef SW_VECTOR_ENGINE_UV_UTILS_H
#define SW_VECTOR_ENGINE_UV_UTILS_H

#include <type_traits>
#include <memory>
#include <chrono>
#include <string>
#include <uv.h>

namespace sw::vengine {
    
class UvError : public Error {
public:
    UvError(const std::string &msg, int err);
};

namespace uv {

template <typename Handle>
inline uv_handle_t* to_handle(Handle *handle) {
    static_assert(std::is_same_v<Handle, uv_handle_t> ||
            std::is_same_v<Handle, uv_stream_t> ||
            std::is_same_v<Handle, uv_tcp_t>);
    return reinterpret_cast<uv_handle_t *>(handle);
}

template <typename Stream>
inline uv_stream_t* to_stream(Stream *stream) {
    static_assert(std::is_same_v<Stream, uv_stream_t> ||
            std::is_same_v<Stream, uv_tcp_t>);
    return reinterpret_cast<uv_stream_t*>(stream);
}

template <typename Handle>
inline void set_handle_data(Handle *handle, void *data) {
    uv_handle_set_data(to_handle(handle), data);
}

template <typename Result, typename Handle>
inline Result* get_handle_data(Handle *handle) {
    auto *data = uv_handle_get_data(to_handle(handle));
    return static_cast<Result *>(data);
}

inline std::string err_msg(int err) {
    std::string name = uv_err_name(err);
    std::string msg = uv_strerror(err);

    return name + ": " msg;
}

struct LoopDeleter {
    void operator()(uv_loop_t *loop) const;
};

using LoopUPtr = std::unique_ptr<uv_loop_t, LoopDeleter>;

LoopUPtr make_loop();

using AsyncUPtr = std::unique_ptr<uv_async_t>;

AsyncUPtr make_async(uv_loop_t &loop, uv_async_cb *callback, void *data = nullptr);

using TcpUPtr = std::unique_ptr<uv_tcp_t>;

struct TcpOptions {
    std::string ip;
    int port;
    int backlog;
    std::chrono::seconds keepalive;
    bool nodelay;
};

TcpUPtr make_tcp(uv_loop_t &loop, const TcpOptions &options);

}

}

#endif // end SW_VECTOR_ENGINE_UV_UTILS_H