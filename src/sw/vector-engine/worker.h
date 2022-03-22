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

#ifndef SW_VECTOR_ENGINE_WORKER_H
#define SW_VECTOR_ENGINE_WORKER_H

#include <vector>
#include <mutex>
#include <atomic>
#include <memory>
#include <thread>
#include <condition_variable>
#include "resp.h"

namespace sw::vengine {

class Reactor;

struct Task {
    std::vector<RespCommand> cmds;

    uint64_t connection_id;

    Reactor *reactor;
};

struct Reply {
    uint64_t connection_id;

    std::string reply;
};

class Worker {
public:
    Worker();

    ~Worker();

    void submit(Task task);

    void stop();

private:
    void _run();

    std::vector<Task> _fetch_tasks();

    Reply _run_task(const Task &task);

    void _run_cmd(const RespCommand &cmd, RespReplyBuilder &builder);

    std::thread _worker;

    std::vector<Task> _tasks;

    std::mutex _mutex;

    std::condition_variable _cv;
};

using WorkerUPtr = std::unique_ptr<Worker>;

class WorkerPool {
public:
    explicit WorkerPool(std::size_t num);

    ~WorkerPool() {
        stop();
    }

    Worker& fetch(uint64_t id);

    void stop();

private:
    std::vector<WorkerUPtr> _workers;
};

using WorkerPoolSPtr = std::shared_ptr<WorkerPool>;

}

#endif // end SW_VECTOR_ENGINE_WORKER_H
