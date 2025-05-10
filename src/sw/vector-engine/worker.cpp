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

#include "sw/vector-engine/worker.h"
#include <cctype>
#include "sw/vector-engine/resp.h"
#include "sw/vector-engine/reactor.h"

namespace sw::vengine {

Worker::Worker() {
    _worker = std::thread([this]() { this->_run(); });
}

Worker::~Worker() {
    stop();

    if (_worker.joinable()) {
        _worker.join();
    }
}

void Worker::submit(BatchTask batch_task) {
    {
        std::lock_guard<std::mutex> lock(_mutex);

        _tasks.push_back(std::move(batch_task));
    }

    _cv.notify_one();
}

void Worker::stop() {
    BatchTask task;
    task.reactor = nullptr;

    submit(std::move(task));
}

void Worker::_run() {
    while (true) {
        auto tasks = _fetch_tasks();

        bool stop_thread = false;
        std::vector<Reply> replies;
        replies.reserve(tasks.size());
        for (auto &batch_task : tasks) {
            if (batch_task.reactor == nullptr) {
                stop_thread = true;
                continue;
            }
            auto reply = _run_batch_task(batch_task);
            replies.emplace_back(std::move(reply));

            batch_task.reactor->send(std::move(replies));
        }

        if (stop_thread) {
            break;
        }
    }
}

std::vector<BatchTask> Worker::_fetch_tasks() {
    std::vector<BatchTask> tasks;
    {
        std::unique_lock<std::mutex> lock(_mutex);

        if (_tasks.empty()) {
            _cv.wait(lock, [this]() { return !(this->_tasks).empty(); });
        }

        tasks.swap(_tasks);
    }

    return tasks;
}

Reply Worker::_run_batch_task(BatchTask &batch_task) {
    Reply reply;
    reply.connection_id = batch_task.connection_id;
    for (const auto &task : batch_task.tasks) {
        auto output = task->run();
        reply.reply += batch_task.response_builder->build(output.get());
    }

    return reply;
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

    auto &worker = _workers[id % _workers.size()];
    if (worker) {
        return *worker;
    } else {
        throw Error("worker has been stopped");
    }
}

void WorkerPool::stop() {
    for (auto &worker : _workers) {
        if (worker) {
            worker->stop();
            worker = nullptr;
        }
    }
}

}
