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

#ifndef SW_VECTOR_ENGINE_JSON_RPC_H
#define SW_VECTOR_ENGINE_JSON_RPC_H

#include <memory>
#include <functional>
#include <string>
#include <string_view>
#include <optional>
#include <unordered_map>
#include <vector>
#include "nlohmann/json.hpp"
#include "sw/vector-engine/protocol.h"

namespace sw::vengine {

class Task;
class TaskOutput;

using JsonRpcReply = std::string;

using JsonRpcRequest = nlohmann::json;

class JsonRpcResponseBuilder : public ResponseBuilder {
public:
    virtual std::string build(TaskOutput *output) override;
};

class JsonRpcRequestParser : public RequestParser {
public:
    virtual auto parse(std::string_view buffer) const
        -> std::pair<std::vector<std::unique_ptr<Task>>, std::size_t> override;
};

}

#endif // end SW_VECTOR_ENGINE_JSON_RPC_H
