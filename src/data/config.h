/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: config.h - Configuration management API
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

#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>
#include <stddef.h>
#include <SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Configuration structure containing all application settings
 */
typedef struct {
    /* Display settings */
    bool fullscreen;
    
    /* Debug settings */
    bool debug;
    char *log_filename;
    int log_level;
    size_t max_file_size;
    int max_files;
    bool log_to_file;
    bool log_to_stderr;
    bool log_overwrite;
    
    /* Game settings */
    int initial_level;
    int max_colors;
    int sound_volume;
    
    /* Internal state */
    char *config_file_path;
    bool modified;
} config_t;

/**
 * Configuration option types for validation
 */
typedef enum {
    CONFIG_TYPE_BOOL,
    CONFIG_TYPE_INT,
    CONFIG_TYPE_STRING,
    CONFIG_TYPE_SIZE_T
} config_type_t;

/**
 * Configuration option definition for validation
 */
typedef struct {
    const char *name;
    config_type_t type;
    union {
        struct {
            bool default_value;
            bool min_value;
            bool max_value;
        } bool_val;
        struct {
            int default_value;
            int min_value;
            int max_value;
        } int_val;
        struct {
            const char *default_value;
            size_t max_length;
        } string_val;
        struct {
            size_t default_value;
            size_t min_value;
            size_t max_value;
        } size_val;
    } validation;
} config_option_t;

/**
 * Initialize configuration with default values
 * @return Pointer to initialized config structure (must be freed with config_free)
 */
config_t* config_init(void);

/**
 * Load configuration from file
 * @param config Configuration structure to populate
 * @param filename Path to configuration file
 * @return true on success, false on failure
 */
bool config_load(config_t *config, const char *filename);

/**
 * Save configuration to file
 * @param config Configuration structure to save
 * @param filename Path to configuration file (NULL to use current path)
 * @return true on success, false on failure
 */
bool config_save(const config_t *config, const char *filename);

/**
 * Create or update configuration file with missing options
 * @param filename Path to configuration file
 * @return true on success, false on failure
 */
bool config_create_or_update(const char *filename);

/**
 * Get default configuration file path
 * @return Path to default configuration file (must be freed by caller)
 */
char* config_get_default_path(void);

/**
 * Parse command line arguments and update configuration
 * @param config Configuration structure to update
 * @param argc Argument count
 * @param argv Argument vector
 * @return Number of arguments consumed
 */
int config_parse_args(config_t *config, int argc, char *argv[]);

/**
 * Validate configuration values
 * @param config Configuration structure to validate
 * @return true if valid, false if invalid
 */
bool config_validate(const config_t *config);

/**
 * Get configuration value as string (for display/debugging)
 * @param config Configuration structure
 * @param option_name Name of option to get
 * @return String representation of value (must be freed by caller)
 */
char* config_get_string(const config_t *config, const char *option_name);

/**
 * Set configuration value from string
 * @param config Configuration structure
 * @param option_name Name of option to set
 * @param value String value to set
 * @return true on success, false on failure
 */
bool config_set_string(config_t *config, const char *option_name, const char *value);

/**
 * Free configuration structure and all allocated memory
 * @param config Configuration structure to free
 */
void config_free(config_t *config);

/**
 * Print configuration to stdout (for debugging)
 * @param config Configuration structure to print
 */
void config_print(const config_t *config);

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_H */ 