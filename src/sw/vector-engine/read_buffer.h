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

#ifndef SW_VECTOR_ENGINE_READ_BUFFER_H
#define SW_VECTOR_ENGINE_READ_BUFFER_H

#include <string>
#include <string_view>

namespace sw::vengine {

class ReadBuffer {
public:
    ReadBuffer(std::size_t min_size, std::size_t max_size);

    std::pair<char*, std::size_t> alloc(std::size_t suggested_size);

    void dealloc(std::size_t size);

    void occupy(std::size_t size) noexcept;

    std::string_view data() {
        return {_buf.data(), _size};
    }

private:
    std::size_t _realloc_if_needed(std::size_t suggested_size);

    std::size_t _min_size;
    std::size_t _max_size;
    std::size_t _size;
    std::string _buf;
};

}

#endif // end SW_VECTOR_ENGINE_READ_BUFFER_H
