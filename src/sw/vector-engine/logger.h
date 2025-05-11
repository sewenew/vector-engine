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

#ifndef SW_VECTOR_ENGINE_LOGGER_H
#define SW_VECTOR_ENGINE_LOGGER_H

#include <cassert>
#include <stdarg.h>
#include <string>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "sw/vector-engine/errors.h"

namespace sw::vengine {

enum class LogLevel {
    DEBUG = SPDLOG_LEVEL_DEBUG,
    INFO = SPDLOG_LEVEL_INFO,
    WARN = SPDLOG_LEVEL_WARN,
    ERROR = SPDLOG_LEVEL_ERROR,
    CRITICAL = SPDLOG_LEVEL_CRITICAL
};

struct LoggerOptions {
    std::string path = "logs/vector-engine.log";
    std::size_t max_size = 5 * 1024 * 1024;
    std::size_t max_files = 3;
    LogLevel level = LogLevel::INFO;
    std::string pattern = "[%Y-%m-%d %H:%M:%S.%e] [%n] [%t] [%l] [%s:%#] %v";
};

class Logger {
public:
    static Logger& instance();

    Logger(const Logger &) = delete;
    Logger& operator=(const Logger &) = delete;
    Logger(Logger &&) = delete;
    Logger& operator=(Logger &&) = delete;

    void init(const LoggerOptions &opts);

    spdlog::logger* get() {
        assert(_logger);
        return _logger.get();
    }

private:
    Logger() = default;

    std::shared_ptr<spdlog::logger> _logger;
};

#define VECTOR_ENGINE_DEBUG(...) \
    SPDLOG_LOGGER_DEBUG(sw::vengine::Logger::instance().get(), __VA_ARGS__)

#define VECTOR_ENGINE_INFO(...) \
    SPDLOG_LOGGER_INFO(sw::vengine::Logger::instance().get(), __VA_ARGS__)

#define VECTOR_ENGINE_WARN(...) \
    SPDLOG_LOGGER_WARN(sw::vengine::Logger::instance().get(), __VA_ARGS__)

#define VECTOR_ENGINE_ERROR(...) \
    SPDLOG_LOGGER_ERROR(sw::vengine::Logger::instance().get(), __VA_ARGS__)

#define VECTOR_ENGINE_CRITICAL(...) \
    SPDLOG_LOGGER_CRITICAL(sw::vengine::Logger::instance().get(), __VA_ARGS__)

}

#endif // end SW_VECTOR_ENGINE_LOGGER_H
