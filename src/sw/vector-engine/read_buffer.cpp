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

#include "read_buffer.h"
#include "errors.h"

namespace sw::vengine {

ReadBuffer::ReadBuffer(std::size_t min_size, std::size_t max_size) :
    _min_size(min_size),
    _max_size(max_size) {
    assert(_min_size > 0 && _min_size <= _max_size) {

    _size = 0;
    _buf.resize(_min_size);
}

std::pair<char*, std::size_t> ReadBuffer::alloc(std::size_t suggested_size) {
    auto remain = _realloc_if_needed(suggested_size);

    return std::make_pair(_buf.data() + _size, remain);
}

void ReadBuffer::occupy(std::size_t size) noexcept {
    assert(size + _size <= _buf.size());

    _size += size;
}

void ReadBuffer::dealloc(std::size_t size) {
    assert(size > 0 && size <= _size);

    if (_size - size < _min_size / 2) {
        std::string buf(_min_size, '\0');
        std::copy(_buf.data() + size, _buf.data() + _size, buf.data());
        _buf.swap(buf);
    } else {
        std::copy(_buf.data() + size, _buf.data() + _size, _buf.data());
    }

    _size -= size;
}

std::size_t ReadBuffer::_realloc_if_needed(std::size_t suggested_size) {
    auto capacity = _buf.size();
    assert(capacity >= _size);

    auto remain = capacity - _size;

    while (remain < suggested_size) {
        if (capacity >= _max_size) {
            // Already reach the max size limit.
            break;
        }

        capacity *= 2;
        if (capacity > _max_size) {
            capacity = _max_size;
        }

        remain = capacity - _size;
    }

    if (capacity > _buf.size()) {
        _buf.resize(capacity);
    }

    return remain;
}

}
