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

#include "sw/vector-engine/logger.h"

namespace sw::vengine {

Logger& Logger::instance() {
    static Logger inst;
    return inst;
}

void Logger::init(const LoggerOptions &opts) {
    auto l = spdlog::rotating_logger_mt("vector-engine", opts.path, opts.max_size, opts.max_files);

    auto level = spdlog::level::debug;
    switch (opts.level) {
    case LogLevel::DEBUG:
        level = spdlog::level::debug;
    case LogLevel::INFO:
        level = spdlog::level::info;
        break;
    case LogLevel::WARN:
        level = spdlog::level::warn;
        break;
    case LogLevel::ERROR:
        level = spdlog::level::err;
        break;
    case LogLevel::CRITICAL:
        level = spdlog::level::critical;
        break;
    default:
        throw Error("unsupported log level");
    }

    l->set_level(level);

    if (!opts.pattern.empty()) {
        l->set_pattern(opts.pattern);
    }

    _logger = l;
}

}
