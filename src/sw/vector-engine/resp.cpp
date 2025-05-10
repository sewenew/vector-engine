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

#include "sw/vector-engine/resp.h"
#include "sw/vector-engine/errors.h"
#include "sw/vector-engine/task.h"
#include "sw/vector-engine/ping_task.h"
#include "sw/vector-engine/unknown_task.h"
#include "sw/vector-engine/string_utils.h"
#include <cassert>
#include <charconv>

namespace sw::vengine {

auto RespCommandParser::parse(std::string_view buffer) const
    -> std::pair<std::vector<RespCommand>, std::size_t> {
    auto *first = buffer.data();
    std::vector<RespCommand> cmds;
    std::size_t bytes_parsed = 0;

    while (true) {
        auto argc = _parse_argc(buffer);
        if (!argc) {
            // Incomplete request.
            break;
        }

        if (*argc == 0) {
            throw Error("invalid request: zero argument");
        }

        auto name = _parse_argv(buffer);
        if (!name) {
            // Incomplete request.
            break;
        }

        std::vector<std::string> args;
        args.reserve(*argc - 1);
        auto idx = 0U;
        for ( ; idx != *argc - 1; ++idx) {
            auto argv = _parse_argv(buffer);
            if (!argv) {
                // Incomplete request.
                break;
            }
            args.push_back(std::move(*argv));
        }

        if (idx < *argc - 1) {
            // Incomplete request.
            break;
        }

        RespCommand cmd;
        cmd.name = *name;
        cmd.args = std::move(args);
        cmds.push_back(std::move(cmd));
        bytes_parsed = (buffer.data() - first);
    }

    return std::make_pair(std::move(cmds), bytes_parsed);
}

std::optional<std::size_t> RespCommandParser::_parse_num(char c, std::string_view &buffer) const {
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

std::optional<std::string> RespCommandParser::_parse_argv(std::string_view &buffer) const {
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

RespReplyBuilder& RespReplyBuilder::append_bulk_string(const std::string_view &str) {
    // $size\r\nstr\r\n
    auto len = std::to_string(str.size());
    std::string reply;
    reply.reserve(1 + len.size() + 2 + str.size() + 2);
    reply.push_back('$');
    reply.insert(reply.end(), len.data(), len.data() + len.size());
    reply += "\r\n";
    reply.insert(reply.end(), str.data(), str.data() + str.size());
    reply += "\r\n";

    _buffer += reply;

    return *this;
}

RespReplyBuilder& RespReplyBuilder::_append_string(char type, const std::string_view &str) {
    std::string reply;
    reply.reserve(1 + str.size() + 2);
    reply.push_back(type);
    reply.insert(reply.end(), str.data(), str.data() + str.size());
    reply += "\r\n";

    _buffer += reply;

    return *this;
}

std::string RespResponseBuilder::build(TaskOutput *output) {
    assert(output != nullptr);

    return output->to_resp_reply();
}

auto RespRequestParser::parse(std::string_view buffer) const
    -> std::pair<std::vector<TaskUPtr>, std::size_t> {
    RespCommandParser parser;
    auto [cmds, len] = parser.parse(buffer);

    RespTaskCreator creator;
    std::vector<TaskUPtr> tasks;
    for (auto &cmd : cmds) {
        auto task = creator.create(cmd);
        tasks.push_back(std::move(task));
    }

    return std::make_pair(std::move(tasks), len);
}

const RespTaskCreator::CreatorMap RespTaskCreator::_creators = {
    {"ping", create_resp_task<PingTask>}
};

TaskUPtr RespTaskCreator::create(RespCommand cmd) {
    auto name = string_utils::to_lower(cmd.name);
    auto iter = _creators.find(name);
    if (iter == _creators.end()) {
        return _make_unknown_task(std::move(cmd));
    }

    return iter->second(std::move(cmd));
}

TaskUPtr RespTaskCreator::_make_unknown_task(RespCommand cmd) const {
    auto unknown_task = std::make_unique<UnknownTask>();
    unknown_task->from_resp_command(std::move(cmd));
    return unknown_task;
}

}
