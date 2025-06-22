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

#include "sw/vector-engine/json_rpc.h"
#include "sw/vector-engine/errors.h"
#include "sw/vector-engine/task.h"
#include <cassert>

namespace sw::vengine {

auto JsonRpcRequestParser::parse(std::string_view buffer) const
    -> std::pair<std::vector<TaskUPtr>, std::size_t> {
    return std::make_pair({}, 0);
}

std::string JsonRpcResponseBuilder::build(TaskOutput *output) {
    assert(output != nullptr);

    return output->to_mcp_reply();
}

}
