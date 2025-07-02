#pragma once

#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>

enum class LogLevel {
    LOG_DEBUG = 0,
    LOG_INFO = 1,
    LOG_ERROR = 2
};

class Logger {
public:
    static LogLevel current_level;
    
    static std::string get_timestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
        return ss.str();
    }
    
    static std::string level_to_string(LogLevel level) {
        switch (level) {
            case LogLevel::LOG_DEBUG: return "DEBUG";
            case LogLevel::LOG_INFO:  return "INFO ";
            case LogLevel::LOG_ERROR: return "ERROR";
            default: return "UNKNOWN";
        }
    }
};

// Static member will be defined in main.cpp to avoid multiple definitions
// LogLevel Logger::current_level = LogLevel::INFO;

#define LOG(level, msg) \
    do { \
        if (static_cast<int>(level) >= static_cast<int>(Logger::current_level)) { \
            std::cerr << "[" << Logger::get_timestamp() << "] " \
                      << "[" << Logger::level_to_string(level) << "] " \
                      << msg << std::endl; \
        } \
    } while(0)

#define LOG_DEBUG(msg) LOG(LogLevel::LOG_DEBUG, msg)
#define LOG_INFO(msg)  LOG(LogLevel::LOG_INFO, msg)
#define LOG_ERROR(msg) LOG(LogLevel::LOG_ERROR, msg)
