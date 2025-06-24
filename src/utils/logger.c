#include "utils/logger.h"
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <time.h>
#include <stdio.h>
#include <SDL2/SDL.h>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

/* Global logger configuration */
logger_config_t logger_config = {
    .enabled = false,
    .log_to_file = false,
    .log_to_stderr = true,
    .log_overwrite = false,
    .level = LOG_LEVEL_DEBUG,
    .log_filename = NULL,
    .max_file_size = 0,
    .max_files = 5,
#ifdef HAVE_ZLOG
    .zlog_cat = NULL
#endif
};

/* Backward compatibility variable */
bool debug_enabled = false;

/* Log level names for output */
static const char* level_names[] = {
    "ERROR",
    "WARN", 
    "INFO",
    "DEBUG",
    "TRACE"
};

/* Timestamp buffer */
static char timestamp_buffer[64];

/* Helper: get executable directory */
static char* get_executable_dir(void) {
    static char exec_path[1024];
    static char *exec_dir = NULL;
    
    if (exec_dir == NULL) {
#ifdef __APPLE__
        uint32_t size = sizeof(exec_path);
        if (_NSGetExecutablePath(exec_path, &size) == 0) {
            exec_dir = dirname(exec_path);
        } else {
            exec_dir = ".";
        }
#else
        ssize_t count = readlink("/proc/self/exe", exec_path, sizeof(exec_path));
        if (count != -1) {
            exec_dir = dirname(exec_path);
        } else {
            exec_dir = ".";
        }
#endif
    }
    return exec_dir;
}

/* Helper: get current timestamp */
static const char* get_timestamp(void) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    
    strftime(timestamp_buffer, sizeof(timestamp_buffer), "%Y-%m-%d %H:%M:%S", tm_info);
    return timestamp_buffer;
}

#ifdef HAVE_ZLOG
/* Create zlog configuration string */
static char* create_zlog_config(const char *log_filename, bool log_to_stderr, 
                               size_t max_file_size, int max_files, bool log_overwrite) {
    char *config = malloc(4096);
    if (!config) return NULL;
    
    char full_log_path[1024];
    snprintf(full_log_path, sizeof(full_log_path), "%s/%s", get_executable_dir(), log_filename);
    
    snprintf(config, 4096,
        "[global]\n"
        "strict init = true\n"
        "buffer min = 1024\n"
        "buffer max = 2MB\n"
        "rotate lock file = /tmp/zlog.lock\n"
        "file perms = 600\n"
        "\n"
        "[formats]\n"
        "simple = \"%%d(%%ms) [%%V] [%%F:%%L:%%N] %%m%%n\"\n"
        "\n"
        "[rules]\n"
        "quadromania.* %s \"%s\", %zu, %d, \"simple\"; simple\n"
        "%squadromania.* stderr \"simple\"; simple\n",
        log_overwrite ? "w" : "a",
        full_log_path,
        max_file_size > 0 ? max_file_size : 1024*1024,
        max_files,
        log_to_stderr ? "" : "#"
    );
    
    return config;
}
#endif

bool logger_init(bool enable, bool log_to_file, bool log_to_stderr, 
                log_level_t level, const char *log_filename, 
                size_t max_file_size, int max_files, bool log_overwrite)
{
    logger_config.enabled = enable;
    logger_config.log_to_file = log_to_file;
    logger_config.log_to_stderr = log_to_stderr;
    logger_config.level = level;
    logger_config.max_file_size = max_file_size;
    logger_config.max_files = max_files;
    logger_config.log_overwrite = log_overwrite;
    
    /* Set backward compatibility variable */
    debug_enabled = enable;
    
    if (!enable) {
        return true;
    }
    
#ifdef HAVE_ZLOG
    /* Initialize zlog */
    int rc = zlog_init(NULL);
    if (rc) {
        fprintf(stderr, "zlog init failed\n");
        return false;
    }
    
    /* Create zlog category */
    logger_config.zlog_cat = zlog_get_category("quadromania");
    if (!logger_config.zlog_cat) {
        fprintf(stderr, "zlog get category failed\n");
        zlog_fini();
        return false;
    }
    
    /* If file logging is requested, create zlog configuration */
    if (log_to_file && log_filename) {
        logger_config.log_filename = strdup(log_filename);
        
        char *zlog_config = create_zlog_config(log_filename, log_to_stderr, 
                                              max_file_size, max_files, log_overwrite);
        if (!zlog_config) {
            fprintf(stderr, "Failed to create zlog configuration\n");
            return false;
        }
        
        /* Write config to temporary file */
        char config_file[256];
        snprintf(config_file, sizeof(config_file), "/tmp/quadromania_zlog.conf");
        FILE *f = fopen(config_file, "w");
        if (f) {
            fwrite(zlog_config, 1, strlen(zlog_config), f);
            fclose(f);
            
            /* Reinitialize zlog with our config */
            zlog_fini();
            rc = zlog_init(config_file);
            if (rc) {
                fprintf(stderr, "zlog init with config failed\n");
                free(zlog_config);
                unlink(config_file);
                return false;
            }
            
            logger_config.zlog_cat = zlog_get_category("quadromania");
            if (!logger_config.zlog_cat) {
                fprintf(stderr, "zlog get category failed after config\n");
                zlog_fini();
                free(zlog_config);
                unlink(config_file);
                return false;
            }
            
            unlink(config_file);
        }
        
        free(zlog_config);
    }
    
    LOG_INFO("Logger initialized with zlog - Level: %d, File: %s, Stderr: %s, Overwrite: %s", 
             level, 
             log_to_file ? (log_filename ? log_filename : "none") : "disabled",
             log_to_stderr ? "enabled" : "disabled",
             log_overwrite ? "enabled" : "disabled");
#else
    /* Fallback logging without zlog */
    if (log_to_file && log_filename) {
        logger_config.log_filename = strdup(log_filename);
    }
    
    fprintf(stderr, "[INFO] Logger initialized (fallback mode) - Level: %s, File: %s, Stderr: %s\n",
            level_names[level],
            log_to_file ? (log_filename ? log_filename : "none") : "disabled",
            log_to_stderr ? "enabled" : "disabled");
#endif
    
    return true;
}

void logger_init_simple(bool enable)
{
    logger_init(enable, false, true, LOG_LEVEL_DEBUG, NULL, 0, 5, false);
    if (enable) {
        LOG_INFO("Logger enabled (simple initialization)");
    }
}

void logger_cleanup(void)
{
    if (logger_config.log_filename) {
        LOG_INFO("Logger shutting down");
        free(logger_config.log_filename);
        logger_config.log_filename = NULL;
    }
    
#ifdef HAVE_ZLOG
    if (logger_config.zlog_cat) {
        zlog_fini();
        logger_config.zlog_cat = NULL;
    }
#endif
    
    logger_config.enabled = false;
    debug_enabled = false;
}

bool logger_is_enabled(log_level_t level)
{
    return logger_config.enabled && level <= logger_config.level;
}

void logger_log(log_level_t level, const char *file, int line, const char *func, 
                const char *fmt, ...)
{
    if (!logger_is_enabled(level)) {
        return;
    }
    
#ifdef HAVE_ZLOG
    if (!logger_config.zlog_cat) {
        return;
    }
    
    /* Prepare the log message */
    va_list args;
    char message_buffer[1024];
    
    va_start(args, fmt);
    vsnprintf(message_buffer, sizeof(message_buffer), fmt, args);
    va_end(args);
    
    /* Extract filename from path */
    const char *filename = strrchr(file, '/');
    filename = filename ? filename + 1 : file;
    
    /* Log with zlog */
    switch (level) {
        case LOG_LEVEL_ERROR:
            zlog_error(logger_config.zlog_cat, "[%s:%d:%s] %s", filename, line, func, message_buffer);
            break;
        case LOG_LEVEL_WARN:
            zlog_warn(logger_config.zlog_cat, "[%s:%d:%s] %s", filename, line, func, message_buffer);
            break;
        case LOG_LEVEL_INFO:
            zlog_info(logger_config.zlog_cat, "[%s:%d:%s] %s", filename, line, func, message_buffer);
            break;
        case LOG_LEVEL_DEBUG:
            zlog_debug(logger_config.zlog_cat, "[%s:%d:%s] %s", filename, line, func, message_buffer);
            break;
        case LOG_LEVEL_TRACE:
            zlog_debug(logger_config.zlog_cat, "[TRACE][%s:%d:%s] %s", filename, line, func, message_buffer);
            break;
    }
#else
    /* Fallback logging without zlog */
    va_list args;
    char message_buffer[1024];
    
    va_start(args, fmt);
    vsnprintf(message_buffer, sizeof(message_buffer), fmt, args);
    va_end(args);
    
    /* Extract filename from path */
    const char *filename = strrchr(file, '/');
    filename = filename ? filename + 1 : file;
    
    /* Format log line */
    char log_line[2048];
    snprintf(log_line, sizeof(log_line), "[%s] [%s] [%s:%d:%s] %s\n",
             get_timestamp(), level_names[level], filename, line, func, message_buffer);
    
    /* Output to stderr if enabled */
    if (logger_config.log_to_stderr) {
        fprintf(stderr, "%s", log_line);
        fflush(stderr);
    }
    
    /* Output to file if enabled */
    if (logger_config.log_to_file && logger_config.log_filename) {
        FILE *log_file = fopen(logger_config.log_filename, 
                              logger_config.log_overwrite ? "w" : "a");
        if (log_file) {
            fprintf(log_file, "%s", log_line);
            fflush(log_file);
            fclose(log_file);
        }
    }
#endif
} 