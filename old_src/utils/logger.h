#ifndef LOGGER_H
#define LOGGER_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Log levels - map to spdlog levels */
typedef enum {
    LOG_LEVEL_ERROR = 0,
    LOG_LEVEL_WARN = 1,
    LOG_LEVEL_INFO = 2,
    LOG_LEVEL_DEBUG = 3,
    LOG_LEVEL_TRACE = 4
} log_level_t;

/* Initialize logger with spdlog */
bool logger_init(bool enable, bool log_to_file, bool log_to_stderr, 
                log_level_t level, const char *log_filename, 
                size_t max_file_size, int max_files, bool log_overwrite);

/* Core logging function */
void logger_log(log_level_t level, const char *file, int line, const char *func, 
                const char *fmt, ...);

#ifdef __cplusplus
}
#endif

/* Log level macros */
#define LOG_ERROR(fmt, ...) \
    logger_log(LOG_LEVEL_ERROR, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

#define LOG_WARN(fmt, ...) \
    logger_log(LOG_LEVEL_WARN, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

#define LOG_INFO(fmt, ...) \
    logger_log(LOG_LEVEL_INFO, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

#define LOG_DEBUG(fmt, ...) \
    logger_log(LOG_LEVEL_DEBUG, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

#define LOG_TRACE(fmt, ...) \
    logger_log(LOG_LEVEL_TRACE, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

/* Backward compatibility macros */
#define DEBUG_PRINT(fmt, ...) \
    LOG_DEBUG(fmt, ##__VA_ARGS__)

#endif /* LOGGER_H */ 