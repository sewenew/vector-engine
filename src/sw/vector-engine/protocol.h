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

#ifndef SW_VECTOR_ENGINE_PROTOCOL_H
#define SW_VECTOR_ENGINE_PROTOCOL_H

#include <memory>
#include <string>

namespace sw::vengine {

class TaskOutput;
class Task;

enum class ProtocolType {
    RESP = 0
};

struct ProtocolOptions {
    ProtocolType type;
};

class ResponseBuilder {
public:
    virtual ~ResponseBuilder() = default;

    virtual std::string build(TaskOutput *output) = 0;
};
using ResponseBuilderUPtr = std::unique_ptr<ResponseBuilder>;

class ResponseBuilderCreator {
public:
    ResponseBuilderUPtr create(ProtocolType type) const;
};

class RequestParser {
public:
    virtual ~RequestParser() = default;

    virtual auto parse(std::string_view buffer) const
        -> std::pair<std::vector<std::unique_ptr<Task>>, std::size_t> = 0;
};
using RequestParserUPtr = std::unique_ptr<RequestParser>;

class RequestParserCreator {
public:
    RequestParserUPtr create(ProtocolType type) const;
};

}

#endif // end SW_VECTOR_ENGINE_PROTOCOL_H
