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

#include "worker.h"
#include <cctype>
#include "resp.h"
#include "reactor.h"

namespace {

std::string to_lower(const std::string &str) {
    std::string res;
    res.reserve(str.size());
    for (auto c : str) {
        res.push_back(std::tolower(c));
    }

    return res;
}

}

namespace sw::vengine {

Worker::Worker() {
    _worker = std::thread([this]() { this->_run(); });
}

Worker::~Worker() {
    _stop = true;

    if (_worker.joinable()) {
        _worker.join();
    }
}

void Worker::submit(Task task) {
    {
        std::lock_guard<std::mutex> lock(_mutex);

        _tasks.push_back(std::move(task));
    }

    _cv.notify_one();
}

void Worker::_run() {
    while (!_stop) {
        auto tasks = _fetch_tasks();

        for (auto &task : tasks) {
            // TODO: if there's a null task, we should break the loop.
            auto reply = _run_task(std::move(task));
            _send_reply(std::move(reply));
        }
    }
}

std::vector<Task> Worker::_fetch_tasks() {
    std::vector<Task> tasks;
    {
        std::unique_lock<std::mutex> lock(_mutex);

        if (_tasks.empty()) {
            _cv.wait(lock, [this]() { return !(this->_tasks).empty(); });
        }

        tasks.swap(_tasks);
    }

    return tasks;
}

Reply Worker::_run_task(Task task) {
    Reply reply;
    reply.connection_id = task.connection_id;
    reply.reactor = task.reactor;
    RespReplyBuilder builder;
    for (const auto &cmd : task.cmds) {
        _run_cmd(cmd, builder);
    }

    reply.reply = std::move(builder.data());

    return reply;
}

void Worker::_run_cmd(const RespCommand &cmd, RespReplyBuilder &builder) {
    const auto &args = cmd.args;
    if (args.empty()) {
        builder.append_error("empty arguments");
        return;
    }

    auto name = to_lower(args.front());
    if (name == "command") {
        builder.append_array(1);
        builder.append_array(6);
        builder.append_bulk_string("command");
        builder.append_integer(-1);
        builder.append_array(1);
        builder.append_bulk_string("random");
        builder.append_integer(0);
        builder.append_integer(0);
        builder.append_integer(0);
    } else {
        builder.append_error("unknown command: " + name);
    }
}

void Worker::_send_reply(Reply reply) {
    auto *reactor = reply.reactor;

    reactor->send(std::move(reply));
}

WorkerPool::WorkerPool(std::size_t num) {
    if (num == 0) {
        throw Error("size of worker pool must larger than 0");
    }

    _workers.reserve(num);
    for (auto idx = 0U; idx != num; ++idx) {
        _workers.push_back(std::make_unique<Worker>());
    }
}

Worker& WorkerPool::fetch(uint64_t id) {
    assert(_workers.size() > 0);

    return *(_workers[id % _workers.size()]);
}

}
