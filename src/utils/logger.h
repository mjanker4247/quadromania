#ifndef LOGGER_H
#define LOGGER_H

#include "common/datatypes.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef HAVE_SPDLOG
#include <spdlog/spdlog.h>
#endif

/* Log levels - map to zlog levels */
typedef enum {
    LOG_LEVEL_ERROR = 0,
    LOG_LEVEL_WARN = 1,
    LOG_LEVEL_INFO = 2,
    LOG_LEVEL_DEBUG = 3,
    LOG_LEVEL_TRACE = 4
} log_level_t;

/* Logger configuration structure */
typedef struct {
    bool enabled;
    bool log_to_file;
    bool log_to_stderr;
    bool log_overwrite;
    log_level_t level;
    char *log_filename;
    size_t max_file_size;
    int max_files;
#ifdef HAVE_SPDLOG
    spdlog_logger_t *spdlog_logger;
#endif
} logger_config_t;

extern logger_config_t logger_config;
extern bool debug_enabled;  /* Backward compatibility */

/* Initialize logger with zlog */
bool logger_init(bool enable, bool log_to_file, bool log_to_stderr, 
                log_level_t level, const char *log_filename, 
                size_t max_file_size, int max_files, bool log_overwrite);

/* Initialize logger with simple boolean (backward compatibility) */
void logger_init_simple(bool enable);

/* Close logger and cleanup resources */
void logger_cleanup(void);

/* Check if logger is enabled for a specific level */
bool logger_is_enabled(log_level_t level);

/* Core logging function */
void logger_log(log_level_t level, const char *file, int line, const char *func, 
                const char *fmt, ...);

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
    do { \
        if (debug_enabled) { \
            logger_log(LOG_LEVEL_DEBUG, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__); \
        } \
    } while(0)

#define DEBUG_INIT(enable) \
    do { \
        logger_init_simple(enable); \
    } while(0)

#endif /* LOGGER_H */ 