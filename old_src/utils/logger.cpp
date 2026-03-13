#include "utils/logger.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <memory>
#include <vector>
#include <cstdarg>
#include <cstdio>
#include <cstring>

static std::shared_ptr<spdlog::logger> g_logger = nullptr;

static spdlog::level::level_enum to_spdlog_level(log_level_t level) {
    switch (level) {
        case LOG_LEVEL_ERROR: return spdlog::level::err;
        case LOG_LEVEL_WARN:  return spdlog::level::warn;
        case LOG_LEVEL_INFO:  return spdlog::level::info;
        case LOG_LEVEL_DEBUG: return spdlog::level::debug;
        case LOG_LEVEL_TRACE: return spdlog::level::trace;
        default:              return spdlog::level::info;
    }
}

extern "C" bool logger_init(bool enable, bool log_to_file, bool log_to_stderr,
                            log_level_t level, const char *log_filename,
                            size_t max_file_size, int max_files, bool log_overwrite) {
    if (!enable) {
        g_logger = nullptr;
        return true;
    }
    try {
        std::vector<spdlog::sink_ptr> sinks;
        if (log_to_stderr) {
            auto stderr_sink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
            stderr_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%s:%#:%!] %v");
            sinks.push_back(stderr_sink);
        }
        if (log_to_file && log_filename) {
            std::string path = log_filename;
            spdlog::sink_ptr file_sink;
            if (max_file_size > 0 && max_files > 1) {
                file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(path, max_file_size, max_files, log_overwrite);
            } else {
                file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path, log_overwrite);
            }
            file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%s:%#:%!] %v");
            sinks.push_back(file_sink);
        }
        if (!sinks.empty()) {
            g_logger = std::make_shared<spdlog::logger>("quadromania", sinks.begin(), sinks.end());
        } else {
            g_logger = spdlog::stdout_color_mt("quadromania");
        }
        g_logger->set_level(to_spdlog_level(level));
        spdlog::set_default_logger(g_logger);
        return true;
    } catch (...) {
        g_logger = nullptr;
        return false;
    }
}

extern "C" void logger_log(log_level_t level, const char *file, int line, const char *func, const char *fmt, ...) {
    if (!g_logger) return;
    char message[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(message, sizeof(message), fmt, args);
    va_end(args);
    const char *filename = strrchr(file, '/');
    filename = filename ? filename + 1 : file;
    switch (level) {
        case LOG_LEVEL_ERROR:
            g_logger->error("[{}:{}:{}] {}", filename, line, func, message);
            break;
        case LOG_LEVEL_WARN:
            g_logger->warn("[{}:{}:{}] {}", filename, line, func, message);
            break;
        case LOG_LEVEL_INFO:
            g_logger->info("[{}:{}:{}] {}", filename, line, func, message);
            break;
        case LOG_LEVEL_DEBUG:
            g_logger->debug("[{}:{}:{}] {}", filename, line, func, message);
            break;
        case LOG_LEVEL_TRACE:
            g_logger->trace("[{}:{}:{}] {}", filename, line, func, message);
            break;
        default:
            g_logger->info("[{}:{}:{}] {}", filename, line, func, message);
            break;
    }
} 