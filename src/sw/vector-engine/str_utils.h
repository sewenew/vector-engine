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

#ifndef SW_VECTOR_ENGINE_STR_UTILS_H
#define SW_VECTOR_ENGINE_STR_UTILS_H

#include <algorithm>
#include <string>
#include <string_view>

namespace sw::vengine::str {

std::string to_lower(const std::string_view &str);

std::string_view trim(const std::string_view &str);

std::string_view trim_left(const std::string_view &str);

std::string_view trim_right(const std::string_view &str);

bool starts_with(const std::string_view &str, const std::string_view &prefix);

bool ends_with(const std::string_view &str, const std::string_view &postfix);

}

#endif // end SW_VECTOR_ENGINE_STR_UTILS_H
