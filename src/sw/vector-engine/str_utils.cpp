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

#include "sw/vector-engine/str_utils.h"
#include <cctype>
#include <cstring>

namespace sw::vengine::str {

std::string to_lower(const std::string_view &str) {
    std::string res;
    res.reserve(str.size());
    for (auto c : str) {
        res.push_back(std::tolower(c));
    }

    return res;
}

std::string_view trim(const std::string_view &line) {
    auto str = trim_left(line);
    str = trim_right(str);
    return str;
}

std::string_view trim_left(const std::string_view &line) {
    auto left_iter = line.begin();
    for (; left_iter != line.end(); ++left_iter) {
        if (!std::isspace(static_cast<unsigned char>(*left_iter))) {
            break;
        }
    }

    auto right_iter = line.rbegin();

    return std::string_view(left_iter, right_iter.base() - left_iter);
}

std::string_view trim_right(const std::string_view &line) {
    auto left_iter = line.begin();

    auto right_iter = line.rbegin();
    for (; right_iter != line.rend(); ++right_iter) {
        if (!std::isspace(static_cast<unsigned char>(*right_iter))) {
            break;
        }
    }

    return std::string_view(left_iter, right_iter.base() - left_iter);
}

bool starts_with(const std::string_view &str, const std::string_view &prefix) {
    if (str.size() < prefix.size()) {
        return false;
    }

    return std::memcmp(str.data(), prefix.data(), prefix.size()) == 0;
}

bool ends_with(const std::string_view &str, const std::string_view &postfix) {
    if (str.size() < postfix.size()) {
        return false;
    }

    return std::memcmp(str.data() + str.size() - postfix.size(), postfix.data(), postfix.size()) == 0;
}

}
