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

#ifndef SW_VECTOR_ENGINE_RESP_H
#define SW_VECTOR_ENGINE_RESP_H

#include <memory>
#include <functional>
#include <string>
#include <string_view>
#include <optional>
#include <unordered_map>
#include <vector>
#include "sw/vector-engine/protocol.h"

namespace sw::vengine {

class Task;
class TaskOutput;

using RespArgs = std::vector<std::string>;

struct RespCommand {
    std::string name;
    RespArgs args;
};

class RespCommandParser {
public:
    // @return pair<vector<RespCommand>, number of bytes parsed>
    auto parse(std::string_view data) const
        -> std::pair<std::vector<RespCommand>, std::size_t>;

private:
    std::optional<std::size_t> _parse_num(char c, std::string_view &data) const;

    std::optional<std::size_t> _parse_argc(std::string_view &data) const {
        // *n\r\n
        return _parse_num('*', data);
    }

    std::optional<std::string> _parse_argv(std::string_view &data) const;
};

using RespReply = std::string;

class RespReplyBuilder {
public:
    RespReplyBuilder& append_ok() {
        _buffer += "+OK\r\n";
        return *this;
    }

    RespReplyBuilder& append_simple_string(const std::string_view &str) {
        // +str\r\n
        return _append_string('+', str);
    }

    RespReplyBuilder& append_error(const std::string_view &err) {
        // -err\r\n
        return _append_string('-', err);
    }

    RespReplyBuilder& append_integer(long long num) {
        // :num\r\n
        return _append_string(':', std::to_string(num));
    }

    RespReplyBuilder& append_bulk_string(const std::string_view &str);

    RespReplyBuilder& append_nil() {
        // $-1\r\n
        _buffer += "$-1\r\n";
        return *this;
    }

    RespReplyBuilder& append_array(long long size) {
        // *num\r\n
        return _append_string('*', std::to_string(size));
    }

    RespReply& data() {
        return _buffer;
    }

private:
    RespReplyBuilder& _append_string(char type, const std::string_view &str);

    std::string _buffer;
};

class RespResponseBuilder : public ResponseBuilder {
public:
    virtual std::string build(TaskOutput *output) override;
};

class RespRequestParser : public RequestParser {
public:
    virtual auto parse(std::string_view buffer) const
        -> std::pair<std::vector<std::unique_ptr<Task>>, std::size_t> override;
};

template <typename T>
std::unique_ptr<Task> create_resp_task(RespCommand cmd) {
    auto task = std::make_unique<T>();
    task->from_resp_command(std::move(cmd));

    return task;
}

class RespTaskCreator {
public:
    std::unique_ptr<Task> create(RespCommand cmd);

private:
    std::unique_ptr<Task> _make_unknown_task(RespCommand cmd) const;

    // map<type, creator>
    using CreatorMap = std::unordered_map<std::string, std::function<std::unique_ptr<Task> (RespCommand)>>;
    static const CreatorMap _creators;
};

}

#endif // end SW_VECTOR_ENGINE_RESP_H
