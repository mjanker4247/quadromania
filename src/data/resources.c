/*
 * Quadromania
 * (c) 2002/2003/2009/2010 by Matthias Arndt <marndt@asmsoftware.de> / ASM Software
 *
 * File: resources.c - Centralized resource management system implementation
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

#include "data/resources.h"
#include "utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

/**************************
 * GLOBAL DATA STRUCTURES *
 **************************/

/* Resource management system state */
static struct
{
    bool initialized;
    ResourceCacheConfig config;
    Resource* resource_list;
    Resource* resource_tail;
    uint32_t total_resources;
    uint32_t loaded_resources;
    uint32_t total_memory_usage;
    uint32_t cache_hits;
    uint32_t cache_misses;
    uint32_t load_errors;
    char error_message[512];
    
    /* Custom loaders */
    struct {
        Resource* (*loader)(const char*, const ResourceLoadOptions*);
        void (*unloader)(Resource*);
    } custom_loaders[RESOURCE_TYPE_MAX];
} resource_state = {0};

/* Supported file extensions for each resource type */
static const char* image_extensions[] = {
    ".png", ".jpg", ".jpeg", ".bmp", ".tga", ".gif", ".webp", NULL
};

static const char* sound_extensions[] = {
    ".wav", ".ogg", ".mp3", ".flac", ".aiff", ".mod", ".xm", ".s3m", NULL
};

static const char* music_extensions[] = {
    ".ogg", ".mp3", ".flac", ".wav", ".mod", ".xm", ".s3m", ".it", NULL
};

static const char* font_extensions[] = {
    ".ttf", ".otf", ".fnt", ".bdf", ".pcf", NULL
};

static const char* text_extensions[] = {
    ".txt", ".cfg", ".ini", ".json", ".xml", ".html", ".css", ".js", NULL
};

static const char* binary_extensions[] = {
    ".dat", ".bin", ".pak", ".res", ".arc", NULL
};

static const char** resource_extensions[] = {
    NULL,  /* UNKNOWN */
    image_extensions,
    sound_extensions,
    music_extensions,
    font_extensions,
    text_extensions,
    binary_extensions
};

/*************
 * FUNCTIONS *
 *************/

/**
 * Get default resource cache configuration
 */
ResourceCacheConfig Resources_GetDefaultConfig(void)
{
    ResourceCacheConfig config = {
        .max_memory_mb = 256,
        .max_resources = 1000,
        .cache_timeout_ms = 300000,  /* 5 minutes */
        .enable_compression = false,
        .enable_preloading = true,
        .data_directory = "data"
    };
    return config;
}

/**
 * Validate resource cache configuration
 */
bool Resources_ValidateConfig(const ResourceCacheConfig* config)
{
    if (!config)
    {
        return false;
    }
    
    if (config->max_memory_mb == 0 || config->max_resources == 0)
    {
        return false;
    }
    
    if (strlen(config->data_directory) == 0)
    {
        return false;
    }
    
    return true;
}

/**
 * Initialize the resource management system
 */
bool Resources_Init(const ResourceCacheConfig* config)
{
    if (resource_state.initialized)
    {
        DEBUG_PRINT("Resource management system already initialized");
        return true;
    }

    /* Use default configuration if none provided */
    if (!config)
    {
        resource_state.config = Resources_GetDefaultConfig();
    }
    else
    {
        if (!Resources_ValidateConfig(config))
        {
            LOG_ERROR("Invalid resource cache configuration");
            return false;
        }
        memcpy(&resource_state.config, config, sizeof(ResourceCacheConfig));
    }

    /* Initialize state */
    resource_state.resource_list = NULL;
    resource_state.resource_tail = NULL;
    resource_state.total_resources = 0;
    resource_state.loaded_resources = 0;
    resource_state.total_memory_usage = 0;
    resource_state.cache_hits = 0;
    resource_state.cache_misses = 0;
    resource_state.load_errors = 0;
    memset(resource_state.error_message, 0, sizeof(resource_state.error_message));

    /* Initialize custom loaders */
    memset(resource_state.custom_loaders, 0, sizeof(resource_state.custom_loaders));

    resource_state.initialized = true;
    LOG_INFO("Resource management system initialized successfully");
    
    return true;
}

/**
 * Shutdown the resource management system
 */
void Resources_Shutdown(void)
{
    if (!resource_state.initialized)
    {
        return;
    }

    /* Unload all resources */
    Resources_UnloadAll();

    /* Clear state */
    memset(&resource_state, 0, sizeof(resource_state));
    LOG_INFO("Resource management system shutdown complete");
}

/**
 * Get resource type from file extension
 */
ResourceType Resources_GetTypeFromExtension(const char* extension)
{
    if (!extension)
    {
        return RESOURCE_TYPE_UNKNOWN;
    }

    /* Skip leading dot if present */
    if (extension[0] == '.')
    {
        extension++;
    }

    /* Check each resource type */
    for (int type = RESOURCE_TYPE_IMAGE; type < RESOURCE_TYPE_MAX; type++)
    {
        const char** extensions = resource_extensions[type];
        if (!extensions)
        {
            continue;
        }

        for (int i = 0; extensions[i]; i++)
        {
            const char* ext = extensions[i];
            if (ext[0] == '.')
            {
                ext++;
            }
            
            if (strcasecmp(extension, ext) == 0)
            {
                return (ResourceType)type;
            }
        }
    }

    return RESOURCE_TYPE_UNKNOWN;
}

/**
 * Get resource type from filename
 */
ResourceType Resources_GetTypeFromFilename(const char* filename)
{
    if (!filename)
    {
        return RESOURCE_TYPE_UNKNOWN;
    }

    /* Find the last dot in the filename */
    const char* extension = strrchr(filename, '.');
    if (!extension)
    {
        return RESOURCE_TYPE_UNKNOWN;
    }

    return Resources_GetTypeFromExtension(extension);
}

/**
 * Get supported file extensions for a resource type
 */
const char** Resources_GetSupportedExtensions(ResourceType type)
{
    if (type <= RESOURCE_TYPE_UNKNOWN || type >= RESOURCE_TYPE_MAX)
    {
        return NULL;
    }

    return resource_extensions[type];
}

/**
 * Create a new resource structure
 */
static Resource* Resources_CreateResource(const char* filename, ResourceType type)
{
    Resource* resource = malloc(sizeof(Resource));
    if (!resource)
    {
        LOG_ERROR("Failed to allocate memory for resource");
        return NULL;
    }

    /* Initialize metadata */
    memset(&resource->metadata, 0, sizeof(ResourceMetadata));
    strncpy(resource->metadata.filename, filename, sizeof(resource->metadata.filename) - 1);
    resource->metadata.type = type;
    resource->metadata.state = RESOURCE_STATE_UNLOADED;
    resource->metadata.last_accessed = (uint32_t)time(NULL);
    resource->metadata.access_count = 0;
    resource->metadata.persistent = false;
    resource->metadata.preloaded = false;

    /* Initialize data */
    memset(&resource->data, 0, sizeof(ResourceData));

    /* Initialize linked list pointers */
    resource->next = NULL;
    resource->prev = NULL;

    return resource;
}

/**
 * Add resource to the linked list
 */
static void Resources_AddToList(Resource* resource)
{
    if (!resource)
    {
        return;
    }

    if (!resource_state.resource_list)
    {
        resource_state.resource_list = resource;
        resource_state.resource_tail = resource;
    }
    else
    {
        resource->prev = resource_state.resource_tail;
        resource_state.resource_tail->next = resource;
        resource_state.resource_tail = resource;
    }

    resource_state.total_resources++;
}

/**
 * Remove resource from the linked list
 */
static void Resources_RemoveFromList(Resource* resource)
{
    if (!resource)
    {
        return;
    }

    if (resource->prev)
    {
        resource->prev->next = resource->next;
    }
    else
    {
        resource_state.resource_list = resource->next;
    }

    if (resource->next)
    {
        resource->next->prev = resource->prev;
    }
    else
    {
        resource_state.resource_tail = resource->prev;
    }

    resource_state.total_resources--;
}

/**
 * Find resource by filename
 */
Resource* Resources_Find(const char* filename)
{
    if (!resource_state.initialized || !filename)
    {
        return NULL;
    }

    Resource* current = resource_state.resource_list;
    while (current)
    {
        if (strcmp(current->metadata.filename, filename) == 0)
        {
            /* Update access statistics */
            current->metadata.last_accessed = (uint32_t)time(NULL);
            current->metadata.access_count++;
            resource_state.cache_hits++;
            return current;
        }
        current = current->next;
    }

    resource_state.cache_misses++;
    return NULL;
}

/**
 * Load image resource
 */
static Resource* Resources_LoadImageInternal(const char* filename, const ResourceLoadOptions* options)
{
    Resource* resource = Resources_CreateResource(filename, RESOURCE_TYPE_IMAGE);
    if (!resource)
    {
        return NULL;
    }

    /* Build full path */
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s/%s", 
             resource_state.config.data_directory, filename);

    /* Load surface */
    resource->data.image.surface = IMG_Load(full_path);
    if (!resource->data.image.surface)
    {
        snprintf(resource_state.error_message, sizeof(resource_state.error_message),
                "Failed to load image: %s (SDL_image error: %s)", 
                full_path, IMG_GetError());
        LOG_ERROR("%s", resource_state.error_message);
        free(resource);
        return NULL;
    }

    /* Get image properties */
    resource->data.image.width = resource->data.image.surface->w;
    resource->data.image.height = resource->data.image.surface->h;
    resource->data.image.format = resource->data.image.surface->format;

    /* Create texture if renderer is available */
    // TODO: Get renderer from graphics system
    // resource->data.image.texture = SDL_CreateTextureFromSurface(renderer, resource->data.image.surface);

    /* Update metadata */
    resource->metadata.state = RESOURCE_STATE_LOADED;
    resource->metadata.size = (uint32_t)(resource->data.image.width * resource->data.image.height * 
                                        resource->data.image.surface->format->BytesPerPixel);

    return resource;
}

/**
 * Load sound resource
 */
static Resource* Resources_LoadSoundInternal(const char* filename, const ResourceLoadOptions* options)
{
    Resource* resource = Resources_CreateResource(filename, RESOURCE_TYPE_SOUND);
    if (!resource)
    {
        return NULL;
    }

    /* Build full path */
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s/%s", 
             resource_state.config.data_directory, filename);

    /* Load sound chunk */
    resource->data.sound.chunk = Mix_LoadWAV(full_path);
    if (!resource->data.sound.chunk)
    {
        snprintf(resource_state.error_message, sizeof(resource_state.error_message),
                "Failed to load sound: %s (SDL_mixer error: %s)", 
                full_path, Mix_GetError());
        LOG_ERROR("%s", resource_state.error_message);
        free(resource);
        return NULL;
    }

    /* Set default properties */
    resource->data.sound.volume = 128;
    resource->data.sound.loop = false;
    resource->data.sound.duration_ms = 0;  /* TODO: Calculate duration */

    /* Update metadata */
    resource->metadata.state = RESOURCE_STATE_LOADED;
    resource->metadata.size = (uint32_t)resource->data.sound.chunk->alen;

    return resource;
}

/**
 * Load music resource
 */
static Resource* Resources_LoadMusicInternal(const char* filename, const ResourceLoadOptions* options)
{
    Resource* resource = Resources_CreateResource(filename, RESOURCE_TYPE_MUSIC);
    if (!resource)
    {
        return NULL;
    }

    /* Build full path */
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s/%s", 
             resource_state.config.data_directory, filename);

    /* Load music */
    resource->data.music.music = Mix_LoadMUS(full_path);
    if (!resource->data.music.music)
    {
        snprintf(resource_state.error_message, sizeof(resource_state.error_message),
                "Failed to load music: %s (SDL_mixer error: %s)", 
                full_path, Mix_GetError());
        LOG_ERROR("%s", resource_state.error_message);
        free(resource);
        return NULL;
    }

    /* Set default properties */
    resource->data.music.volume = 128;
    resource->data.music.loop = false;
    resource->data.music.duration_ms = 0;  /* TODO: Calculate duration */

    /* Update metadata */
    resource->metadata.state = RESOURCE_STATE_LOADED;
    resource->metadata.size = 0;  /* TODO: Get file size */

    return resource;
}

/**
 * Load font resource
 */
static Resource* Resources_LoadFontInternal(const char* filename, int size, const ResourceLoadOptions* options)
{
    Resource* resource = Resources_CreateResource(filename, RESOURCE_TYPE_FONT);
    if (!resource)
    {
        return NULL;
    }

    /* Build full path */
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s/%s", 
             resource_state.config.data_directory, filename);

    /* Load font */
    resource->data.font.font = TTF_OpenFont(full_path, size);
    if (!resource->data.font.font)
    {
        snprintf(resource_state.error_message, sizeof(resource_state.error_message),
                "Failed to load font: %s (SDL_ttf error: %s)", 
                full_path, TTF_GetError());
        LOG_ERROR("%s", resource_state.error_message);
        free(resource);
        return NULL;
    }

    /* Set font properties */
    resource->data.font.size = size;
    resource->data.font.bold = false;
    resource->data.font.italic = false;

    /* Update metadata */
    resource->metadata.state = RESOURCE_STATE_LOADED;
    resource->metadata.size = 0;  /* TODO: Get file size */

    return resource;
}

/**
 * Load text resource
 */
static Resource* Resources_LoadTextInternal(const char* filename, const char* encoding, const ResourceLoadOptions* options)
{
    Resource* resource = Resources_CreateResource(filename, RESOURCE_TYPE_TEXT);
    if (!resource)
    {
        return NULL;
    }

    /* Build full path */
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s/%s", 
             resource_state.config.data_directory, filename);

    /* Open file */
    FILE* file = fopen(full_path, "rb");
    if (!file)
    {
        snprintf(resource_state.error_message, sizeof(resource_state.error_message),
                "Failed to open text file: %s", full_path);
        LOG_ERROR("%s", resource_state.error_message);
        free(resource);
        return NULL;
    }

    /* Get file size */
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size <= 0)
    {
        snprintf(resource_state.error_message, sizeof(resource_state.error_message),
                "Invalid file size for text file: %s", full_path);
        LOG_ERROR("%s", resource_state.error_message);
        fclose(file);
        free(resource);
        return NULL;
    }

    /* Allocate memory and read file */
    resource->data.text.text = malloc(file_size + 1);
    if (!resource->data.text.text)
    {
        snprintf(resource_state.error_message, sizeof(resource_state.error_message),
                "Failed to allocate memory for text file: %s", full_path);
        LOG_ERROR("%s", resource_state.error_message);
        fclose(file);
        free(resource);
        return NULL;
    }

    size_t bytes_read = fread(resource->data.text.text, 1, file_size, file);
    fclose(file);

    if (bytes_read != (size_t)file_size)
    {
        snprintf(resource_state.error_message, sizeof(resource_state.error_message),
                "Failed to read text file: %s", full_path);
        LOG_ERROR("%s", resource_state.error_message);
        free(resource->data.text.text);
        free(resource);
        return NULL;
    }

    /* Null-terminate the text */
    resource->data.text.text[file_size] = '\0';
    resource->data.text.length = (uint32_t)file_size;
    strncpy(resource->data.text.encoding, encoding ? encoding : "UTF-8", 
            sizeof(resource->data.text.encoding) - 1);

    /* Update metadata */
    resource->metadata.state = RESOURCE_STATE_LOADED;
    resource->metadata.size = (uint32_t)file_size;

    return resource;
}

/**
 * Load binary resource
 */
static Resource* Resources_LoadBinaryInternal(const char* filename, const char* format, const ResourceLoadOptions* options)
{
    Resource* resource = Resources_CreateResource(filename, RESOURCE_TYPE_BINARY);
    if (!resource)
    {
        return NULL;
    }

    /* Build full path */
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s/%s", 
             resource_state.config.data_directory, filename);

    /* Open file */
    FILE* file = fopen(full_path, "rb");
    if (!file)
    {
        snprintf(resource_state.error_message, sizeof(resource_state.error_message),
                "Failed to open binary file: %s", full_path);
        LOG_ERROR("%s", resource_state.error_message);
        free(resource);
        return NULL;
    }

    /* Get file size */
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size <= 0)
    {
        snprintf(resource_state.error_message, sizeof(resource_state.error_message),
                "Invalid file size for binary file: %s", full_path);
        LOG_ERROR("%s", resource_state.error_message);
        fclose(file);
        free(resource);
        return NULL;
    }

    /* Allocate memory and read file */
    resource->data.binary.data = malloc(file_size);
    if (!resource->data.binary.data)
    {
        snprintf(resource_state.error_message, sizeof(resource_state.error_message),
                "Failed to allocate memory for binary file: %s", full_path);
        LOG_ERROR("%s", resource_state.error_message);
        fclose(file);
        free(resource);
        return NULL;
    }

    size_t bytes_read = fread(resource->data.binary.data, 1, file_size, file);
    fclose(file);

    if (bytes_read != (size_t)file_size)
    {
        snprintf(resource_state.error_message, sizeof(resource_state.error_message),
                "Failed to read binary file: %s", full_path);
        LOG_ERROR("%s", resource_state.error_message);
        free(resource->data.binary.data);
        free(resource);
        return NULL;
    }

    /* Set binary properties */
    resource->data.binary.size = (uint32_t)file_size;
    strncpy(resource->data.binary.format, format ? format : "raw", 
            sizeof(resource->data.binary.format) - 1);

    /* Update metadata */
    resource->metadata.state = RESOURCE_STATE_LOADED;
    resource->metadata.size = (uint32_t)file_size;

    return resource;
}

/**
 * Load a resource from file
 */
Resource* Resources_Load(const char* filename, ResourceType type, const ResourceLoadOptions* options)
{
    if (!resource_state.initialized || !filename)
    {
        return NULL;
    }

    /* Check if resource is already loaded */
    Resource* existing = Resources_Find(filename);
    if (existing)
    {
        if (options && options->force_reload)
        {
            Resources_Unload(existing);
        }
        else
        {
            return existing;
        }
    }

    /* Auto-detect type if not specified */
    if (type == RESOURCE_TYPE_UNKNOWN)
    {
        type = Resources_GetTypeFromFilename(filename);
        if (type == RESOURCE_TYPE_UNKNOWN)
        {
            snprintf(resource_state.error_message, sizeof(resource_state.error_message),
                    "Could not determine resource type for: %s", filename);
            LOG_ERROR("%s", resource_state.error_message);
            return NULL;
        }
    }

    /* Check for custom loader */
    if (resource_state.custom_loaders[type].loader)
    {
        Resource* resource = resource_state.custom_loaders[type].loader(filename, options);
        if (resource)
        {
            Resources_AddToList(resource);
            resource_state.loaded_resources++;
        }
        return resource;
    }

    /* Use built-in loaders */
    Resource* resource = NULL;
    switch (type)
    {
    case RESOURCE_TYPE_IMAGE:
        resource = Resources_LoadImageInternal(filename, options);
        break;
    case RESOURCE_TYPE_SOUND:
        resource = Resources_LoadSoundInternal(filename, options);
        break;
    case RESOURCE_TYPE_MUSIC:
        resource = Resources_LoadMusicInternal(filename, options);
        break;
    case RESOURCE_TYPE_FONT:
        resource = Resources_LoadFontInternal(filename, 16, options);  /* Default size */
        break;
    case RESOURCE_TYPE_TEXT:
        resource = Resources_LoadTextInternal(filename, "UTF-8", options);
        break;
    case RESOURCE_TYPE_BINARY:
        resource = Resources_LoadBinaryInternal(filename, "raw", options);
        break;
    default:
        snprintf(resource_state.error_message, sizeof(resource_state.error_message),
                "Unsupported resource type: %d", type);
        LOG_ERROR("%s", resource_state.error_message);
        return NULL;
    }

    if (resource)
    {
        Resources_AddToList(resource);
        resource_state.loaded_resources++;
        LOG_INFO("Loaded resource: %s (type: %d)", filename, type);
    }
    else
    {
        resource_state.load_errors++;
    }

    return resource;
}

/**
 * Load an image resource
 */
Resource* Resources_LoadImage(const char* filename, const ResourceLoadOptions* options)
{
    return Resources_Load(filename, RESOURCE_TYPE_IMAGE, options);
}

/**
 * Load a sound resource
 */
Resource* Resources_LoadSound(const char* filename, const ResourceLoadOptions* options)
{
    return Resources_Load(filename, RESOURCE_TYPE_SOUND, options);
}

/**
 * Load a music resource
 */
Resource* Resources_LoadMusic(const char* filename, const ResourceLoadOptions* options)
{
    return Resources_Load(filename, RESOURCE_TYPE_MUSIC, options);
}

/**
 * Load a font resource
 */
Resource* Resources_LoadFont(const char* filename, int size, const ResourceLoadOptions* options)
{
    if (!resource_state.initialized || !filename)
    {
        return NULL;
    }

    /* Check if resource is already loaded */
    Resource* existing = Resources_Find(filename);
    if (existing && existing->metadata.type == RESOURCE_TYPE_FONT)
    {
        if (options && options->force_reload)
        {
            Resources_Unload(existing);
        }
        else
        {
            return existing;
        }
    }

    Resource* resource = Resources_LoadFontInternal(filename, size, options);
    if (resource)
    {
        Resources_AddToList(resource);
        resource_state.loaded_resources++;
        LOG_INFO("Loaded font: %s (size: %d)", filename, size);
    }
    else
    {
        resource_state.load_errors++;
    }

    return resource;
}

/**
 * Load a text resource
 */
Resource* Resources_LoadText(const char* filename, const char* encoding, const ResourceLoadOptions* options)
{
    if (!resource_state.initialized || !filename)
    {
        return NULL;
    }

    /* Check if resource is already loaded */
    Resource* existing = Resources_Find(filename);
    if (existing && existing->metadata.type == RESOURCE_TYPE_TEXT)
    {
        if (options && options->force_reload)
        {
            Resources_Unload(existing);
        }
        else
        {
            return existing;
        }
    }

    Resource* resource = Resources_LoadTextInternal(filename, encoding, options);
    if (resource)
    {
        Resources_AddToList(resource);
        resource_state.loaded_resources++;
        LOG_INFO("Loaded text: %s (encoding: %s)", filename, encoding ? encoding : "UTF-8");
    }
    else
    {
        resource_state.load_errors++;
    }

    return resource;
}

/**
 * Load a binary resource
 */
Resource* Resources_LoadBinary(const char* filename, const char* format, const ResourceLoadOptions* options)
{
    if (!resource_state.initialized || !filename)
    {
        return NULL;
    }

    /* Check if resource is already loaded */
    Resource* existing = Resources_Find(filename);
    if (existing && existing->metadata.type == RESOURCE_TYPE_BINARY)
    {
        if (options && options->force_reload)
        {
            Resources_Unload(existing);
        }
        else
        {
            return existing;
        }
    }

    Resource* resource = Resources_LoadBinaryInternal(filename, format, options);
    if (resource)
    {
        Resources_AddToList(resource);
        resource_state.loaded_resources++;
        LOG_INFO("Loaded binary: %s (format: %s)", filename, format ? format : "raw");
    }
    else
    {
        resource_state.load_errors++;
    }

    return resource;
}

/**
 * Get a resource by filename (loads if not already loaded)
 */
Resource* Resources_Get(const char* filename)
{
    if (!resource_state.initialized || !filename)
    {
        return NULL;
    }

    Resource* resource = Resources_Find(filename);
    if (resource)
    {
        return resource;
    }

    /* Try to load the resource */
    return Resources_Load(filename, RESOURCE_TYPE_UNKNOWN, NULL);
}

/**
 * Unload a resource from memory
 */
void Resources_Unload(Resource* resource)
{
    if (!resource_state.initialized || !resource)
    {
        return;
    }

    /* Check for custom unloader */
    if (resource_state.custom_loaders[resource->metadata.type].unloader)
    {
        resource_state.custom_loaders[resource->metadata.type].unloader(resource);
    }
    else
    {
        /* Use built-in unloaders */
        switch (resource->metadata.type)
        {
        case RESOURCE_TYPE_IMAGE:
            if (resource->data.image.surface)
            {
                SDL_FreeSurface(resource->data.image.surface);
                resource->data.image.surface = NULL;
            }
            if (resource->data.image.texture)
            {
                SDL_DestroyTexture(resource->data.image.texture);
                resource->data.image.texture = NULL;
            }
            break;
        case RESOURCE_TYPE_SOUND:
            if (resource->data.sound.chunk)
            {
                Mix_FreeChunk(resource->data.sound.chunk);
                resource->data.sound.chunk = NULL;
            }
            break;
        case RESOURCE_TYPE_MUSIC:
            if (resource->data.music.music)
            {
                Mix_FreeMusic(resource->data.music.music);
                resource->data.music.music = NULL;
            }
            break;
        case RESOURCE_TYPE_FONT:
            if (resource->data.font.font)
            {
                TTF_CloseFont(resource->data.font.font);
                resource->data.font.font = NULL;
            }
            break;
        case RESOURCE_TYPE_TEXT:
            if (resource->data.text.text)
            {
                free(resource->data.text.text);
                resource->data.text.text = NULL;
            }
            break;
        case RESOURCE_TYPE_BINARY:
            if (resource->data.binary.data)
            {
                free(resource->data.binary.data);
                resource->data.binary.data = NULL;
            }
            break;
        default:
            break;
        }
    }

    /* Update memory usage */
    resource_state.total_memory_usage -= resource->metadata.size;
    resource_state.loaded_resources--;

    /* Remove from list and free */
    Resources_RemoveFromList(resource);
    free(resource);

    LOG_INFO("Unloaded resource");
}

/**
 * Unload a resource by filename
 */
void Resources_UnloadByName(const char* filename)
{
    if (!resource_state.initialized || !filename)
    {
        return;
    }

    Resource* resource = Resources_Find(filename);
    if (resource)
    {
        Resources_Unload(resource);
    }
}

/**
 * Unload all resources of a specific type
 */
void Resources_UnloadByType(ResourceType type)
{
    if (!resource_state.initialized)
    {
        return;
    }

    Resource* current = resource_state.resource_list;
    while (current)
    {
        Resource* next = current->next;
        if (current->metadata.type == type)
        {
            Resources_Unload(current);
        }
        current = next;
    }
}

/**
 * Unload all non-persistent resources
 */
void Resources_UnloadAll(void)
{
    if (!resource_state.initialized)
    {
        return;
    }

    Resource* current = resource_state.resource_list;
    while (current)
    {
        Resource* next = current->next;
        if (!current->metadata.persistent)
        {
            Resources_Unload(current);
        }
        current = next;
    }
}

/**
 * Get resource statistics
 */
ResourceStats Resources_GetStats(void)
{
    ResourceStats stats = {0};
    
    if (resource_state.initialized)
    {
        stats.total_resources = resource_state.total_resources;
        stats.loaded_resources = resource_state.loaded_resources;
        stats.cached_resources = resource_state.loaded_resources;
        stats.total_memory_mb = resource_state.total_memory_usage / (1024 * 1024);
        stats.cache_hits = resource_state.cache_hits;
        stats.cache_misses = resource_state.cache_misses;
        stats.load_errors = resource_state.load_errors;
    }
    
    return stats;
}

/**
 * Get resource cache configuration
 */
ResourceCacheConfig Resources_GetConfig(void)
{
    return resource_state.config;
}

/**
 * Set resource cache configuration
 */
bool Resources_SetConfig(const ResourceCacheConfig* config)
{
    if (!resource_state.initialized || !config)
    {
        return false;
    }

    if (!Resources_ValidateConfig(config))
    {
        return false;
    }

    memcpy(&resource_state.config, config, sizeof(ResourceCacheConfig));
    return true;
}

/**
 * Clear resource cache
 */
void Resources_ClearCache(void)
{
    Resources_UnloadAll();
}

/**
 * Get resource memory usage in bytes
 */
uint32_t Resources_GetMemoryUsage(const Resource* resource)
{
    if (!resource)
    {
        return 0;
    }

    return resource->metadata.size;
}

/**
 * Get total memory usage in bytes
 */
uint32_t Resources_GetTotalMemoryUsage(void)
{
    return resource_state.total_memory_usage;
}

/**
 * Check if resource is loaded
 */
bool Resources_IsLoaded(const Resource* resource)
{
    return resource && resource->metadata.state == RESOURCE_STATE_LOADED;
}

/**
 * Register a custom resource loader
 */
bool Resources_RegisterLoader(ResourceType type, 
                             Resource* (*loader_function)(const char*, const ResourceLoadOptions*),
                             void (*unloader_function)(Resource*))
{
    if (!resource_state.initialized || type <= RESOURCE_TYPE_UNKNOWN || type >= RESOURCE_TYPE_MAX)
    {
        return false;
    }

    resource_state.custom_loaders[type].loader = loader_function;
    resource_state.custom_loaders[type].unloader = unloader_function;
    return true;
}

/**
 * Set resource data directory
 */
bool Resources_SetDataDirectory(const char* directory)
{
    if (!resource_state.initialized || !directory)
    {
        return false;
    }

    strncpy(resource_state.config.data_directory, directory, 
            sizeof(resource_state.config.data_directory) - 1);
    return true;
}

/**
 * Get resource data directory
 */
const char* Resources_GetDataDirectory(void)
{
    return resource_state.config.data_directory;
}

/**
 * Reload a resource from disk
 */
bool Resources_Reload(Resource* resource)
{
    if (!resource_state.initialized || !resource)
    {
        return false;
    }

    /* Store original type and filename */
    ResourceType type = resource->metadata.type;
    char filename[256];
    strncpy(filename, resource->metadata.filename, sizeof(filename) - 1);

    /* Unload current resource */
    Resources_Unload(resource);

    /* Reload the resource */
    Resource* new_resource = Resources_Load(filename, type, NULL);
    return new_resource != NULL;
}

/**
 * Reload a resource by filename
 */
bool Resources_ReloadByName(const char* filename)
{
    if (!resource_state.initialized || !filename)
    {
        return false;
    }

    Resource* resource = Resources_Find(filename);
    if (!resource)
    {
        return false;
    }

    return Resources_Reload(resource);
}

/**
 * Get resource error message
 */
const char* Resources_GetErrorMessage(const Resource* resource)
{
    (void)resource;  /* Unused parameter */
    return resource_state.error_message;
}

/**
 * Set resource as persistent
 */
void Resources_SetPersistent(Resource* resource, bool persistent)
{
    if (resource)
    {
        resource->metadata.persistent = persistent;
    }
}

/**
 * Set resource as persistent by filename
 */
void Resources_SetPersistentByName(const char* filename, bool persistent)
{
    if (!resource_state.initialized || !filename)
    {
        return;
    }

    Resource* resource = Resources_Find(filename);
    if (resource)
    {
        Resources_SetPersistent(resource, persistent);
    }
}

/**
 * Preload resources from a list
 */
uint32_t Resources_Preload(const char** filenames, uint32_t count)
{
    if (!resource_state.initialized || !filenames || count == 0)
    {
        return 0;
    }

    uint32_t loaded_count = 0;
    for (uint32_t i = 0; i < count; i++)
    {
        if (filenames[i])
        {
            Resource* resource = Resources_Load(filenames[i], RESOURCE_TYPE_UNKNOWN, NULL);
            if (resource)
            {
                resource->metadata.preloaded = true;
                loaded_count++;
            }
        }
    }

    return loaded_count;
}

/**
 * Preload all resources in a directory
 */
uint32_t Resources_PreloadDirectory(const char* directory, ResourceType type)
{
    /* TODO: Implement directory scanning and preloading */
    (void)directory;
    (void)type;
    return 0;
} 