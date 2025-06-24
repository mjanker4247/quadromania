/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: config.c - Configuration management implementation
 * last Modified: 2024-12-19
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 *
 * THIS SOFTWARE IS SUPPLIED AS IT IS WITHOUT ANY WARRANTY!
 *
 */

#include "data/config.h"
#include "utils/logger.h"
#include "common/version.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <libgen.h>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

/* Configuration option definitions for validation */
static const config_option_t config_options[] = {
    {"fullscreen", CONFIG_TYPE_BOOL, {.bool_val = {false, false, true}}},
    {"debug", CONFIG_TYPE_BOOL, {.bool_val = {false, false, true}}},
    {"log_file", CONFIG_TYPE_STRING, {.string_val = {"quadromania.log", 256}}},
    {"log_level", CONFIG_TYPE_INT, {.int_val = {LOG_LEVEL_INFO, LOG_LEVEL_ERROR, LOG_LEVEL_TRACE}}},
    {"log_max_size", CONFIG_TYPE_SIZE_T, {.size_val = {1024*1024, 1024, 100*1024*1024}}},
    {"log_max_files", CONFIG_TYPE_INT, {.int_val = {5, 1, 100}}},
    {"log_to_stderr", CONFIG_TYPE_BOOL, {.bool_val = {true, false, true}}},
    {"log_overwrite", CONFIG_TYPE_BOOL, {.bool_val = {false, false, true}}},
    {"initial_level", CONFIG_TYPE_INT, {.int_val = {1, 1, 100}}},
    {"max_colors", CONFIG_TYPE_INT, {.int_val = {4, 2, 10}}},
    {"sound_volume", CONFIG_TYPE_INT, {.int_val = {100, 0, 100}}},
    {NULL, CONFIG_TYPE_BOOL, {.bool_val = {false, false, true}}}
};

/* Helper: trim whitespace */
static char *trim(char *str) {
    char *end;
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return str;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

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

/* Helper: find config option by name */
static const config_option_t* find_config_option(const char *name) {
    for (int i = 0; config_options[i].name != NULL; i++) {
        if (strcasecmp(config_options[i].name, name) == 0) {
            return &config_options[i];
        }
    }
    return NULL;
}

/* Helper: parse boolean value */
static bool parse_bool(const char *value) {
    return (strcasecmp(value, "true") == 0 || strcmp(value, "1") == 0);
}

/* Helper: parse log level */
static int parse_log_level(const char *value) {
    if (strcasecmp(value, "error") == 0) return LOG_LEVEL_ERROR;
    if (strcasecmp(value, "warn") == 0) return LOG_LEVEL_WARN;
    if (strcasecmp(value, "info") == 0) return LOG_LEVEL_INFO;
    if (strcasecmp(value, "debug") == 0) return LOG_LEVEL_DEBUG;
    if (strcasecmp(value, "trace") == 0) return LOG_LEVEL_TRACE;
    return LOG_LEVEL_INFO; // default
}

config_t* config_init(void) {
    config_t *config = calloc(1, sizeof(config_t));
    if (!config) return NULL;
    
    /* Set default values */
    config->fullscreen = false;
    config->debug = false;
    config->log_filename = strdup("quadromania.log");
    config->log_level = LOG_LEVEL_INFO;
    config->max_file_size = 1024 * 1024; // 1MB
    config->max_files = 5;
    config->log_to_file = false;
    config->log_to_stderr = true;
    config->log_overwrite = false;
    config->initial_level = 1;
    config->max_colors = 4;
    config->sound_volume = 100;
    config->config_file_path = NULL;
    config->modified = false;
    
    return config;
}

bool config_load(config_t *config, const char *filename) {
    if (!config || !filename) return false;
    
    FILE *f = fopen(filename, "r");
    if (!f) return false;
    
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        char *eq, *key, *val;
        if (line[0] == '#' || line[0] == ';' || line[0] == '\n') continue;
        
        eq = strchr(line, '=');
        if (!eq) continue;
        
        *eq = 0;
        key = trim(line);
        val = trim(eq + 1);
        
        if (strcasecmp(key, "fullscreen") == 0) {
            config->fullscreen = parse_bool(val);
        } else if (strcasecmp(key, "debug") == 0) {
            config->debug = parse_bool(val);
        } else if (strcasecmp(key, "log_file") == 0) {
            if (config->log_filename) free(config->log_filename);
            config->log_filename = strdup(val);
            config->log_to_file = true;
        } else if (strcasecmp(key, "log_level") == 0) {
            config->log_level = parse_log_level(val);
        } else if (strcasecmp(key, "log_max_size") == 0) {
            config->max_file_size = (size_t)atol(val);
        } else if (strcasecmp(key, "log_max_files") == 0) {
            config->max_files = atoi(val);
        } else if (strcasecmp(key, "log_to_stderr") == 0) {
            config->log_to_stderr = parse_bool(val);
        } else if (strcasecmp(key, "log_overwrite") == 0) {
            config->log_overwrite = parse_bool(val);
        } else if (strcasecmp(key, "initial_level") == 0) {
            config->initial_level = atoi(val);
        } else if (strcasecmp(key, "max_colors") == 0) {
            config->max_colors = atoi(val);
        } else if (strcasecmp(key, "sound_volume") == 0) {
            config->sound_volume = atoi(val);
        }
    }
    
    fclose(f);
    
    /* Store config file path */
    if (config->config_file_path) free(config->config_file_path);
    config->config_file_path = strdup(filename);
    
    return true;
}

bool config_save(const config_t *config, const char *filename) {
    if (!config) return false;
    
    const char *save_filename = filename ? filename : config->config_file_path;
    if (!save_filename) return false;
    
    FILE *f = fopen(save_filename, "w");
    if (!f) return false;
    
    fprintf(f, "# Quadromania Configuration File\n");
    fprintf(f, "# Lines starting with # are comments\n\n");
    
    fprintf(f, "# Display settings\n");
    fprintf(f, "fullscreen=%s\n", config->fullscreen ? "true" : "false");
    
    fprintf(f, "\n# Debug settings\n");
    fprintf(f, "debug=%s\n", config->debug ? "true" : "false");
    fprintf(f, "log_file=%s\n", config->log_filename ? config->log_filename : "quadromania.log");
    fprintf(f, "log_level=%s\n", 
        config->log_level == LOG_LEVEL_ERROR ? "error" :
        config->log_level == LOG_LEVEL_WARN ? "warn" :
        config->log_level == LOG_LEVEL_INFO ? "info" :
        config->log_level == LOG_LEVEL_DEBUG ? "debug" : "trace");
    fprintf(f, "log_max_size=%zu\n", config->max_file_size);
    fprintf(f, "log_max_files=%d\n", config->max_files);
    fprintf(f, "log_to_stderr=%s\n", config->log_to_stderr ? "true" : "false");
    fprintf(f, "log_overwrite=%s\n", config->log_overwrite ? "true" : "false");
    
    fprintf(f, "\n# Game settings\n");
    fprintf(f, "initial_level=%d\n", config->initial_level);
    fprintf(f, "max_colors=%d\n", config->max_colors);
    fprintf(f, "sound_volume=%d\n", config->sound_volume);
    
    fclose(f);
    return true;
}

bool config_create_or_update(const char *filename) {
    if (!filename) return false;
    
    FILE *f = fopen(filename, "r");
    bool options_present[12] = {false}; // Track which options are present
    char line[256];
    
    // Read existing config and check which options are present
    if (f) {
        while (fgets(line, sizeof(line), f)) {
            char *eq = strchr(line, '=');
            if (eq) {
                *eq = 0;
                char *key = trim(line);
                for (int i = 0; config_options[i].name != NULL; i++) {
                    if (strcasecmp(key, config_options[i].name) == 0) {
                        options_present[i] = true;
                        break;
                    }
                }
            }
        }
        fclose(f);
    }
    
    // Open file in append mode to add missing options
    f = fopen(filename, "a");
    if (!f) return false;
    
    // Add header if file was empty
    bool has_any = false;
    for (int i = 0; config_options[i].name != NULL; i++) {
        if (options_present[i]) {
            has_any = true;
            break;
        }
    }
    
    if (!has_any) {
        fprintf(f, "# Quadromania Configuration File\n");
        fprintf(f, "# Lines starting with # are comments\n\n");
    }
    
    // Add missing options
    bool added_any = false;
    for (int i = 0; config_options[i].name != NULL; i++) {
        if (!options_present[i]) {
            if (i == 0) fprintf(f, "# Display settings\n");
            else if (i == 2) fprintf(f, "\n# Debug settings\n");
            else if (i == 8) fprintf(f, "\n# Game settings\n");
            
            const config_option_t *opt = &config_options[i];
            switch (opt->type) {
                case CONFIG_TYPE_BOOL:
                    fprintf(f, "%s=%s\n", opt->name, 
                        opt->validation.bool_val.default_value ? "true" : "false");
                    break;
                case CONFIG_TYPE_INT:
                    fprintf(f, "%s=%d\n", opt->name, opt->validation.int_val.default_value);
                    break;
                case CONFIG_TYPE_STRING:
                    fprintf(f, "%s=%s\n", opt->name, opt->validation.string_val.default_value);
                    break;
                case CONFIG_TYPE_SIZE_T:
                    fprintf(f, "%s=%zu\n", opt->name, opt->validation.size_val.default_value);
                    break;
            }
            added_any = true;
        }
    }
    
    fclose(f);
    
    if (added_any) {
        fprintf(stderr, "Updated config file: %s (added missing options)\n", filename);
    } else if (!has_any) {
        fprintf(stderr, "Created new config file: %s\n", filename);
    }
    
    return true;
}

char* config_get_default_path(void) {
    char *default_path = malloc(1024);
    if (!default_path) return NULL;
    
    snprintf(default_path, 1024, "%s/quadromania.cfg", get_executable_dir());
    return default_path;
}

int config_parse_args(config_t *config, int argc, char *argv[]) {
    if (!config || argc < 1) return 0;
    
    int consumed = 0;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--fullscreen") == 0) {
            config->fullscreen = true;
            consumed++;
        } else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--debug") == 0) {
            config->debug = true;
            consumed++;
        } else if (strcmp(argv[i], "--config") == 0 && i + 1 < argc) {
            if (config->config_file_path) free(config->config_file_path);
            config->config_file_path = strdup(argv[i + 1]);
            consumed += 2;
            i++; // Skip next argument
        } else if (strcmp(argv[i], "--log-file") == 0 && i + 1 < argc) {
            if (config->log_filename) free(config->log_filename);
            config->log_filename = strdup(argv[i + 1]);
            config->log_to_file = true;
            consumed += 2;
            i++; // Skip next argument
        } else if (strcmp(argv[i], "--log-level") == 0 && i + 1 < argc) {
            config->log_level = parse_log_level(argv[i + 1]);
            consumed += 2;
            i++; // Skip next argument
        } else if (strcmp(argv[i], "--initial-level") == 0 && i + 1 < argc) {
            config->initial_level = atoi(argv[i + 1]);
            consumed += 2;
            i++; // Skip next argument
        } else if (strcmp(argv[i], "--max-colors") == 0 && i + 1 < argc) {
            config->max_colors = atoi(argv[i + 1]);
            consumed += 2;
            i++; // Skip next argument
        } else if (strcmp(argv[i], "--sound-volume") == 0 && i + 1 < argc) {
            config->sound_volume = atoi(argv[i + 1]);
            consumed += 2;
            i++; // Skip next argument
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            // Print help and exit
            fprintf(stderr, "Quadromania v%s\n", VERSION);
            fprintf(stderr, "Copyright (c) 2002-2010 by Matthias Arndt / ASM Software\n");
            fprintf(stderr, "2024 Modified by Marco Janker\n");
            fprintf(stderr, "This program is free software under the GNU General Public License\n\n");
            fprintf(stderr, "Usage: %s [options]\n", argv[0]);
            fprintf(stderr, "Options:\n");
            fprintf(stderr, "  --config <file>          Load parameters from config file\n");
            fprintf(stderr, "  -f, --fullscreen         Run in fullscreen mode\n");
            fprintf(stderr, "  -d, --debug              Enable debug output (to stderr)\n");
            fprintf(stderr, "      --log-file <file>    Log debug output to file\n");
            fprintf(stderr, "      --log-level <level>  Set log level (error, warn, info, debug, trace)\n");
            fprintf(stderr, "      --initial-level <n>  Set initial game level (1-100)\n");
            fprintf(stderr, "      --max-colors <n>     Set maximum colors (2-10)\n");
            fprintf(stderr, "      --sound-volume <n>   Set sound volume (0-100)\n");
            fprintf(stderr, "  -h, --help               Show this help message\n");
            fprintf(stderr, "\nConfig file format (key=value, one per line):\n");
            fprintf(stderr, "  fullscreen=true|false\n  debug=true|false\n  log_file=quadromania.log\n");
            fprintf(stderr, "  log_level=info|debug|warn|error|trace\n  log_max_size=1000000\n");
            fprintf(stderr, "  log_max_files=3\n  log_to_stderr=true|false\n  log_overwrite=true|false\n");
            fprintf(stderr, "  initial_level=1\n  max_colors=4\n  sound_volume=100\n");
            exit(0);
        } else {
            // Unknown argument, stop parsing
            break;
        }
    }
    
    return consumed;
}

bool config_validate(const config_t *config) {
    if (!config) return false;
    
    for (int i = 0; config_options[i].name != NULL; i++) {
        const config_option_t *opt = &config_options[i];
        
        if (strcasecmp(opt->name, "fullscreen") == 0) {
            if (config->fullscreen < opt->validation.bool_val.min_value || 
                config->fullscreen > opt->validation.bool_val.max_value) {
                return false;
            }
        } else if (strcasecmp(opt->name, "debug") == 0) {
            if (config->debug < opt->validation.bool_val.min_value || 
                config->debug > opt->validation.bool_val.max_value) {
                return false;
            }
        } else if (strcasecmp(opt->name, "log_level") == 0) {
            if (config->log_level < opt->validation.int_val.min_value || 
                config->log_level > opt->validation.int_val.max_value) {
                return false;
            }
        } else if (strcasecmp(opt->name, "log_max_size") == 0) {
            if (config->max_file_size < opt->validation.size_val.min_value || 
                config->max_file_size > opt->validation.size_val.max_value) {
                return false;
            }
        } else if (strcasecmp(opt->name, "log_max_files") == 0) {
            if (config->max_files < opt->validation.int_val.min_value || 
                config->max_files > opt->validation.int_val.max_value) {
                return false;
            }
        } else if (strcasecmp(opt->name, "initial_level") == 0) {
            if (config->initial_level < opt->validation.int_val.min_value || 
                config->initial_level > opt->validation.int_val.max_value) {
                return false;
            }
        } else if (strcasecmp(opt->name, "max_colors") == 0) {
            if (config->max_colors < opt->validation.int_val.min_value || 
                config->max_colors > opt->validation.int_val.max_value) {
                return false;
            }
        } else if (strcasecmp(opt->name, "sound_volume") == 0) {
            if (config->sound_volume < opt->validation.int_val.min_value || 
                config->sound_volume > opt->validation.int_val.max_value) {
                return false;
            }
        }
    }
    
    return true;
}

char* config_get_string(const config_t *config, const char *option_name) {
    if (!config || !option_name) return NULL;
    
    char *result = malloc(256);
    if (!result) return NULL;
    
    if (strcasecmp(option_name, "fullscreen") == 0) {
        snprintf(result, 256, "%s", config->fullscreen ? "true" : "false");
    } else if (strcasecmp(option_name, "debug") == 0) {
        snprintf(result, 256, "%s", config->debug ? "true" : "false");
    } else if (strcasecmp(option_name, "log_file") == 0) {
        snprintf(result, 256, "%s", config->log_filename ? config->log_filename : "");
    } else if (strcasecmp(option_name, "log_level") == 0) {
        snprintf(result, 256, "%s", 
            config->log_level == LOG_LEVEL_ERROR ? "error" :
            config->log_level == LOG_LEVEL_WARN ? "warn" :
            config->log_level == LOG_LEVEL_INFO ? "info" :
            config->log_level == LOG_LEVEL_DEBUG ? "debug" : "trace");
    } else if (strcasecmp(option_name, "log_max_size") == 0) {
        snprintf(result, 256, "%zu", config->max_file_size);
    } else if (strcasecmp(option_name, "log_max_files") == 0) {
        snprintf(result, 256, "%d", config->max_files);
    } else if (strcasecmp(option_name, "log_to_stderr") == 0) {
        snprintf(result, 256, "%s", config->log_to_stderr ? "true" : "false");
    } else if (strcasecmp(option_name, "log_overwrite") == 0) {
        snprintf(result, 256, "%s", config->log_overwrite ? "true" : "false");
    } else if (strcasecmp(option_name, "initial_level") == 0) {
        snprintf(result, 256, "%d", config->initial_level);
    } else if (strcasecmp(option_name, "max_colors") == 0) {
        snprintf(result, 256, "%d", config->max_colors);
    } else if (strcasecmp(option_name, "sound_volume") == 0) {
        snprintf(result, 256, "%d", config->sound_volume);
    } else {
        snprintf(result, 256, "unknown");
    }
    
    return result;
}

bool config_set_string(config_t *config, const char *option_name, const char *value) {
    if (!config || !option_name || !value) return false;
    
    if (strcasecmp(option_name, "fullscreen") == 0) {
        config->fullscreen = parse_bool(value);
    } else if (strcasecmp(option_name, "debug") == 0) {
        config->debug = parse_bool(value);
    } else if (strcasecmp(option_name, "log_file") == 0) {
        if (config->log_filename) free(config->log_filename);
        config->log_filename = strdup(value);
        config->log_to_file = true;
    } else if (strcasecmp(option_name, "log_level") == 0) {
        config->log_level = parse_log_level(value);
    } else if (strcasecmp(option_name, "log_max_size") == 0) {
        config->max_file_size = (size_t)atol(value);
    } else if (strcasecmp(option_name, "log_max_files") == 0) {
        config->max_files = atoi(value);
    } else if (strcasecmp(option_name, "log_to_stderr") == 0) {
        config->log_to_stderr = parse_bool(value);
    } else if (strcasecmp(option_name, "log_overwrite") == 0) {
        config->log_overwrite = parse_bool(value);
    } else if (strcasecmp(option_name, "initial_level") == 0) {
        config->initial_level = atoi(value);
    } else if (strcasecmp(option_name, "max_colors") == 0) {
        config->max_colors = atoi(value);
    } else if (strcasecmp(option_name, "sound_volume") == 0) {
        config->sound_volume = atoi(value);
    } else {
        return false; // Unknown option
    }
    
    config->modified = true;
    return true;
}

void config_free(config_t *config) {
    if (!config) return;
    
    if (config->log_filename) free(config->log_filename);
    if (config->config_file_path) free(config->config_file_path);
    free(config);
}

void config_print(const config_t *config) {
    if (!config) return;
    
    printf("Configuration:\n");
    printf("  Display:\n");
    printf("    fullscreen: %s\n", config->fullscreen ? "true" : "false");
    printf("  Debug:\n");
    printf("    debug: %s\n", config->debug ? "true" : "false");
    printf("    log_file: %s\n", config->log_filename ? config->log_filename : "none");
    printf("    log_level: %d\n", config->log_level);
    printf("    log_max_size: %zu\n", config->max_file_size);
    printf("    log_max_files: %d\n", config->max_files);
    printf("    log_to_stderr: %s\n", config->log_to_stderr ? "true" : "false");
    printf("    log_overwrite: %s\n", config->log_overwrite ? "true" : "false");
    printf("  Game:\n");
    printf("    initial_level: %d\n", config->initial_level);
    printf("    max_colors: %d\n", config->max_colors);
    printf("    sound_volume: %d\n", config->sound_volume);
    printf("  Internal:\n");
    printf("    config_file: %s\n", config->config_file_path ? config->config_file_path : "none");
    printf("    modified: %s\n", config->modified ? "true" : "false");
} 