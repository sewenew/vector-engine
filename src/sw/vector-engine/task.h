/**************************************************************************
   Copyright (c) 2025 sewenew

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

#ifndef SW_VECTOR_ENGINE_TASK_H
#define SW_VECTOR_ENGINE_TASK_H

#include <unordered_map>
#include "sw/vector-engine/connection.h"
#include "sw/vector-engine/resp.h"

namespace sw::vengine {

class TaskOutput {
public:
    virtual ~TaskOutput() = default;

    virtual RespReply to_resp_reply() = 0;
};

using TaskOutputUPtr = std::unique_ptr<TaskOutput>;

class Task {
public:
    virtual ~Task() = default;

    virtual void from_resp_command(RespCommand cmd) = 0;

    virtual TaskOutputUPtr run() = 0;
};

using TaskUPtr = std::unique_ptr<Task>;

}

#endif // end SW_VECTOR_ENGINE_TASK_H
