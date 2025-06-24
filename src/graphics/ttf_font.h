#ifndef _TTF_FONT_H_
#define _TTF_FONT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "common/sysconfig.h"
#include "common/datatypes.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>

// Initialize TTF font system
bool TTF_Font_Init(void);

// Clean up TTF font system
void TTF_Font_CleanUp(void);

// Draw text using TTF font
void TTF_Font_DrawText(SDL_Renderer *renderer, Uint16 x, Uint16 y, char *text);

// Get text width in pixels
Uint16 TTF_Font_GetTextWidth(char *text);

// Get font height in pixels
Uint16 TTF_Font_GetHeight(void);

#ifdef __cplusplus
}
#endif

#endif /* _TTF_FONT_H_ */ 