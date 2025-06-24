/************************************************************************
 *    SFONT - SDL Font Library by Karl Bartel <karlb@gmx.net>		*
 *                                                                       *
 *  All functions are explained below. There are two versions of each    *
 *  funtction. The first is the normal one, the function with the        *
 *  2 at the end can be used when you want to handle more than one font  *
 *  in your program.                                                     *
 *                                                                       *
 ************************************************************************/

#ifndef SFONT_H
#define SFONT_H

#include <SDL2/SDL.h>

extern SDL_Texture *Font;
extern Uint16 FontWidth;
extern Uint16 FontHeight;
extern SDL_Rect CharPos[256];

// Delcare one variable of this type for each font you are using.
// To load the fonts, load the font image into YourFont->Surface
// and call InitFont( YourFont );
typedef struct
{
	SDL_Surface *Surface;
	int CharPos[512];
	int h;
} SFont_FontInfo;

// Initialize the font
void InitFont(SDL_Texture *Font);
void InitFont2(SFont_FontInfo *Font);

// Blits a string to a surface with the font at position x, y.
void PutString(SDL_Renderer *renderer, Uint16 x, Uint16 y, char *Text);
void PutString2(SDL_Renderer *renderer, SFont_FontInfo *Font, int x, int y, char *text);

// Returns the width of "text" in pixels
int TextWidth(char *text);
int TextWidth2(SFont_FontInfo *Font, char *text);

// Blits a string to with center alignment to a surface with the font at position y
void XCenteredString(SDL_Renderer *renderer, Uint16 y, char *Text);
void XCenteredString2(SDL_Renderer *renderer, SFont_FontInfo *Font, int y, char *text);

// Input text at the given position with a maximum width
void SFont_Input(SDL_Renderer *renderer, int x, int y, int Width, char *text);

#endif /* SFONT_H */
