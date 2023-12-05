#ifndef SHRIMP_COMMON_LOGGER_HPP
#define SHRIMP_COMMON_LOGGER_HPP

#include <iostream>
#include <cstdint>

namespace shrimp {

enum LogLevel : uint8_t { NONE = 0, ERROR, INFO, DEBUG };

inline LogLevel getLogLevelByString(std::string str)
{
    if (str == "error") {
        return LogLevel::ERROR;
    }
    if (str == "info") {
        return LogLevel::INFO;
    }
    if (str == "debug") {
        return LogLevel::DEBUG;
    }
    return LogLevel::NONE;
}

}  // namespace shrimp
#define LOG_(os, data) os << data << std::endl;

#define LOG_INFO(data, level)                  \
    if (level == LogLevel::INFO) {             \
        LOG_(std::cout, "LOG[INFO]: " << data) \
    }

#define LOG_ERROR(data, level)                  \
    if (level == LogLevel::ERROR) {             \
        LOG_(std::cout, "LOG[ERROR]: " << data) \
    }

#define LOG_DEBUG(data, level)                  \
    if (level == LogLevel::DEBUG) {             \
        LOG_(std::cout, "LOG[DEBUG]: " << data) \
    }

#endif  // SHRIMP_COMMON_LOGGER_HPP