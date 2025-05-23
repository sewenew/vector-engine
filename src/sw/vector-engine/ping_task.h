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

#ifndef SW_VECTOR_ENGINE_PING_TASK_H
#define SW_VECTOR_ENGINE_PING_TASK_H

#include "sw/vector-engine/task.h"
#include "sw/vector-engine/resp.h"

namespace sw::vengine {

class PingTask : public Task {
public:
    PingTask() = default;

    virtual void from_resp_command(RespCommand /*cmd*/) override {}

    virtual TaskOutputUPtr run() override;
};

class PingTaskOutput : public TaskOutput {
public:
    virtual RespReply to_resp_reply() override;
};

}

#endif // end SW_VECTOR_ENGINE_PING_TASK_H
