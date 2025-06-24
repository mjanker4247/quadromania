/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: resources.h - Centralized resource management system
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

#ifndef __RESOURCES_H
#define __RESOURCES_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdint.h>

/**************************
 * DATA TYPE DECLARATIONS *
 **************************/

/**
 * Resource types supported by the system
 */
typedef enum
{
    RESOURCE_TYPE_UNKNOWN = 0,
    RESOURCE_TYPE_IMAGE,
    RESOURCE_TYPE_SOUND,
    RESOURCE_TYPE_MUSIC,
    RESOURCE_TYPE_FONT,
    RESOURCE_TYPE_TEXT,
    RESOURCE_TYPE_BINARY,
    RESOURCE_TYPE_MAX
} ResourceType;

/**
 * Resource loading states
 */
typedef enum
{
    RESOURCE_STATE_UNLOADED = 0,
    RESOURCE_STATE_LOADING,
    RESOURCE_STATE_LOADED,
    RESOURCE_STATE_ERROR,
    RESOURCE_STATE_UNLOADING
} ResourceState;

/**
 * Resource metadata structure
 */
typedef struct
{
    char filename[256];
    char path[512];
    ResourceType type;
    ResourceState state;
    uint32_t size;
    uint32_t last_accessed;
    uint32_t access_count;
    bool persistent;  /* Don't unload when cache is full */
    bool preloaded;   /* Load at startup */
} ResourceMetadata;

/**
 * Image resource data
 */
typedef struct
{
    SDL_Surface* surface;
    SDL_Texture* texture;
    int width;
    int height;
    SDL_PixelFormat* format;
} ImageResource;

/**
 * Sound resource data
 */
typedef struct
{
    Mix_Chunk* chunk;
    uint32_t duration_ms;
    uint8_t volume;
    bool loop;
} SoundResource;

/**
 * Music resource data
 */
typedef struct
{
    Mix_Music* music;
    uint32_t duration_ms;
    uint8_t volume;
    bool loop;
} MusicResource;

/**
 * Font resource data
 */
typedef struct
{
    TTF_Font* font;
    int size;
    bool bold;
    bool italic;
} FontResource;

/**
 * Text resource data
 */
typedef struct
{
    char* text;
    uint32_t length;
    char encoding[32];
} TextResource;

/**
 * Binary resource data
 */
typedef struct
{
    uint8_t* data;
    uint32_t size;
    char format[32];
} BinaryResource;

/**
 * Union of all resource data types
 */
typedef union
{
    ImageResource image;
    SoundResource sound;
    MusicResource music;
    FontResource font;
    TextResource text;
    BinaryResource binary;
} ResourceData;

/**
 * Complete resource structure
 */
typedef struct Resource
{
    ResourceMetadata metadata;
    ResourceData data;
    struct Resource* next;  /* For linked list management */
    struct Resource* prev;
} Resource;

/**
 * Resource cache configuration
 */
typedef struct
{
    uint32_t max_memory_mb;
    uint32_t max_resources;
    uint32_t cache_timeout_ms;
    bool enable_compression;
    bool enable_preloading;
    char data_directory[512];
} ResourceCacheConfig;

/**
 * Resource loading options
 */
typedef struct
{
    bool force_reload;
    bool keep_in_memory;
    uint32_t priority;
    void* user_data;
} ResourceLoadOptions;

/**
 * Resource statistics
 */
typedef struct
{
    uint32_t total_resources;
    uint32_t loaded_resources;
    uint32_t cached_resources;
    uint32_t total_memory_mb;
    uint32_t cache_hits;
    uint32_t cache_misses;
    uint32_t load_errors;
} ResourceStats;

/**************
 * PROTOTYPES *
 **************/

/**
 * Initialize the resource management system
 * @param config Resource cache configuration
 * @return true on success, false on failure
 */
bool Resources_Init(const ResourceCacheConfig* config);

/**
 * Shutdown the resource management system
 */
void Resources_Shutdown(void);

/**
 * Load a resource from file
 * @param filename Resource filename
 * @param type Resource type (can be UNKNOWN for auto-detection)
 * @param options Loading options
 * @return Resource handle or NULL on failure
 */
Resource* Resources_Load(const char* filename, ResourceType type, const ResourceLoadOptions* options);

/**
 * Load an image resource
 * @param filename Image filename
 * @param options Loading options
 * @return Resource handle or NULL on failure
 */
Resource* Resources_LoadImage(const char* filename, const ResourceLoadOptions* options);

/**
 * Load a sound resource
 * @param filename Sound filename
 * @param options Loading options
 * @return Resource handle or NULL on failure
 */
Resource* Resources_LoadSound(const char* filename, const ResourceLoadOptions* options);

/**
 * Load a music resource
 * @param filename Music filename
 * @param options Loading options
 * @return Resource handle or NULL on failure
 */
Resource* Resources_LoadMusic(const char* filename, const ResourceLoadOptions* options);

/**
 * Load a font resource
 * @param filename Font filename
 * @param size Font size
 * @param options Loading options
 * @return Resource handle or NULL on failure
 */
Resource* Resources_LoadFont(const char* filename, int size, const ResourceLoadOptions* options);

/**
 * Load a text resource
 * @param filename Text filename
 * @param encoding Text encoding (UTF-8, ASCII, etc.)
 * @param options Loading options
 * @return Resource handle or NULL on failure
 */
Resource* Resources_LoadText(const char* filename, const char* encoding, const ResourceLoadOptions* options);

/**
 * Load a binary resource
 * @param filename Binary filename
 * @param format Format identifier
 * @param options Loading options
 * @return Resource handle or NULL on failure
 */
Resource* Resources_LoadBinary(const char* filename, const char* format, const ResourceLoadOptions* options);

/**
 * Get a resource by filename (loads if not already loaded)
 * @param filename Resource filename
 * @return Resource handle or NULL if not found
 */
Resource* Resources_Get(const char* filename);

/**
 * Get a resource by filename without loading
 * @param filename Resource filename
 * @return Resource handle or NULL if not loaded
 */
Resource* Resources_Find(const char* filename);

/**
 * Unload a resource from memory
 * @param resource Resource handle
 */
void Resources_Unload(Resource* resource);

/**
 * Unload a resource by filename
 * @param filename Resource filename
 */
void Resources_UnloadByName(const char* filename);

/**
 * Unload all resources of a specific type
 * @param type Resource type
 */
void Resources_UnloadByType(ResourceType type);

/**
 * Unload all non-persistent resources
 */
void Resources_UnloadAll(void);

/**
 * Preload resources from a list
 * @param filenames Array of filenames to preload
 * @param count Number of filenames
 * @return Number of successfully preloaded resources
 */
uint32_t Resources_Preload(const char** filenames, uint32_t count);

/**
 * Preload all resources in a directory
 * @param directory Directory path
 * @param type Resource type filter (UNKNOWN for all types)
 * @return Number of successfully preloaded resources
 */
uint32_t Resources_PreloadDirectory(const char* directory, ResourceType type);

/**
 * Get resource statistics
 * @return Resource statistics structure
 */
ResourceStats Resources_GetStats(void);

/**
 * Get resource cache configuration
 * @return Current cache configuration
 */
ResourceCacheConfig Resources_GetConfig(void);

/**
 * Set resource cache configuration
 * @param config New cache configuration
 * @return true on success, false on failure
 */
bool Resources_SetConfig(const ResourceCacheConfig* config);

/**
 * Clear resource cache
 */
void Resources_ClearCache(void);

/**
 * Get resource memory usage in bytes
 * @param resource Resource handle
 * @return Memory usage in bytes
 */
uint32_t Resources_GetMemoryUsage(const Resource* resource);

/**
 * Get total memory usage in bytes
 * @return Total memory usage in bytes
 */
uint32_t Resources_GetTotalMemoryUsage(void);

/**
 * Check if resource is loaded
 * @param resource Resource handle
 * @return true if loaded, false otherwise
 */
bool Resources_IsLoaded(const Resource* resource);

/**
 * Get resource type from filename
 * @param filename Resource filename
 * @return Detected resource type
 */
ResourceType Resources_GetTypeFromFilename(const char* filename);

/**
 * Get resource type from file extension
 * @param extension File extension (with or without dot)
 * @return Detected resource type
 */
ResourceType Resources_GetTypeFromExtension(const char* extension);

/**
 * Get supported file extensions for a resource type
 * @param type Resource type
 * @return Array of supported extensions (NULL-terminated)
 */
const char** Resources_GetSupportedExtensions(ResourceType type);

/**
 * Register a custom resource loader
 * @param type Resource type
 * @param loader_function Loader function pointer
 * @param unloader_function Unloader function pointer
 * @return true on success, false on failure
 */
bool Resources_RegisterLoader(ResourceType type, 
                             Resource* (*loader_function)(const char*, const ResourceLoadOptions*),
                             void (*unloader_function)(Resource*));

/**
 * Set resource data directory
 * @param directory Data directory path
 * @return true on success, false on failure
 */
bool Resources_SetDataDirectory(const char* directory);

/**
 * Get resource data directory
 * @return Data directory path
 */
const char* Resources_GetDataDirectory(void);

/**
 * Reload a resource from disk
 * @param resource Resource handle
 * @return true on success, false on failure
 */
bool Resources_Reload(Resource* resource);

/**
 * Reload a resource by filename
 * @param filename Resource filename
 * @return true on success, false on failure
 */
bool Resources_ReloadByName(const char* filename);

/**
 * Get resource error message
 * @param resource Resource handle
 * @return Error message or NULL if no error
 */
const char* Resources_GetErrorMessage(const Resource* resource);

/**
 * Set resource as persistent (won't be unloaded when cache is full)
 * @param resource Resource handle
 * @param persistent true to make persistent, false otherwise
 */
void Resources_SetPersistent(Resource* resource, bool persistent);

/**
 * Set resource as persistent by filename
 * @param filename Resource filename
 * @param persistent true to make persistent, false otherwise
 */
void Resources_SetPersistentByName(const char* filename, bool persistent);

/**
 * Get default resource cache configuration
 * @return Default configuration
 */
ResourceCacheConfig Resources_GetDefaultConfig(void);

/**
 * Validate resource cache configuration
 * @param config Configuration to validate
 * @return true if valid, false otherwise
 */
bool Resources_ValidateConfig(const ResourceCacheConfig* config);

#endif /* __RESOURCES_H */ 