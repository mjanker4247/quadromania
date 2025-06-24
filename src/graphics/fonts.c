#include <SDL.h>
#include <stdlib.h>
#include <string.h>

#include "graphics/fonts.h"
#include "graphics/renderer.h"
#include "utils/logger.h"

static Uint32 GetPixel(SDL_Surface *, Sint32, Sint32);
static SFont_FontInfo InternalFont;
SDL_Texture *Font = NULL;
static int FontH;
static Uint8 *FontMap;
static SDL_Rect FontRect;

/* Global variables declared in header */
Uint16 FontWidth = 0;
Uint16 FontHeight = 0;
SDL_Rect CharPos[256];

static Uint32 GetPixel(SDL_Surface *Surface, Sint32 X, Sint32 Y)
{
	Uint8 *bits;
	Uint32 Bpp;

	if (X < 0)
		puts(
				"SFONT ERROR: x too small in GetPixel. Report this to <karlb@gmx.net>");
	if (X >= Surface->w)
		puts(
				"SFONT ERROR: x too big in GetPixel. Report this to <karlb@gmx.net>");

	Bpp = Surface->format->BytesPerPixel;

	bits = ((Uint8 *) Surface->pixels) + Y * Surface->pitch + X * Bpp;

	// Get the pixel
	switch (Bpp)
	{
	case 1:
		return *((Uint8 *) Surface->pixels + Y * Surface->pitch + X);
		break;
	case 2:
		return *((Uint16 *) Surface->pixels + Y * Surface->pitch / 2 + X);
		break;
	case 3:
	{ // Format/endian independent
		Uint8 r, g, b;
		r = *((bits) + Surface->format->Rshift / 8);
		g = *((bits) + Surface->format->Gshift / 8);
		b = *((bits) + Surface->format->Bshift / 8);
		return SDL_MapRGB(Surface->format, r, g, b);
	}
		break;
	case 4:
		return *((Uint32 *) Surface->pixels + Y * Surface->pitch / 4 + X);
		break;
	}

	return -1;
}

void InitFont2(SFont_FontInfo *Font)
{
	Uint16 x = 0, i = 0;

	if (Font->Surface == NULL)
	{
		printf("The font has not been loaded!\n");
		exit(1);
	}

	if (SDL_MUSTLOCK(Font->Surface))
		SDL_LockSurface(Font->Surface);

	while (x < Font->Surface->w)
	{
		if (GetPixel(Font->Surface, x, 0) == SDL_MapRGB(Font->Surface->format,
				255, 0, 255))
		{
			Font->CharPos[i++] = x;
			while ((x < Font->Surface->w - 1) && (GetPixel(Font->Surface, x, 0)
					== SDL_MapRGB(Font->Surface->format, 255, 0, 255)))
				x++;
			Font->CharPos[i++] = x;
		}
		x++;
	}
	if (SDL_MUSTLOCK(Font->Surface))
		SDL_UnlockSurface(Font->Surface);

	Font->h = Font->Surface->h;
	SDL_SetColorKey(Font->Surface, SDL_TRUE, GetPixel(Font->Surface, 0,
			Font->Surface->h - 1));
}

void InitFont(SDL_Texture *FontTexture)
{
	Uint16 i;
	SDL_Rect src;
	int w, h;
	
	Font = FontTexture;
	SDL_QueryTexture(Font, NULL, NULL, &w, &h);
	FontHeight = h;
	FontWidth = w / 256;  // Each character is 1/256th of the total width
	
	DEBUG_PRINT("Font dimensions: %dx%d (each char: %dx%d)\n", w, h, FontWidth, FontHeight);
	
	for(i = 0; i < 256; i++)
	{
		src.x = i * FontWidth;
		src.y = 0;
		src.w = FontWidth;
		src.h = FontHeight;
		CharPos[i] = src;
	}
}

void PutString2(SDL_Renderer *renderer, SFont_FontInfo *Font, int x, int y,
		char *text)
{
	Uint16 ofs;
	Uint16 i = 0;
	SDL_Rect srcrect, dstrect;
	SDL_Texture *font_texture = SDL_CreateTextureFromSurface(renderer, Font->Surface);

	while (text[i] != '\0')
	{
		if (text[i] == ' ')
		{
			x += Font->CharPos[2] - Font->CharPos[1];
			i++;
		}
		else
		{
			ofs = ((unsigned char) text[i] - 33) * 2 + 1;
			srcrect.w = dstrect.w = (Font->CharPos[ofs + 2] + Font->CharPos[ofs
					+ 1]) / 2 - (Font->CharPos[ofs] + Font->CharPos[ofs - 1])
					/ 2;
			srcrect.h = dstrect.h = Font->Surface->h - 1;
			srcrect.x = (Font->CharPos[ofs] + Font->CharPos[ofs - 1]) / 2;
			srcrect.y = 1;
			dstrect.x = x - (float) (Font->CharPos[ofs]
					- Font->CharPos[ofs - 1]) / 2;
			dstrect.y = y;

			SDL_RenderCopy(renderer, font_texture, &srcrect, &dstrect);

			x += Font->CharPos[ofs + 1] - Font->CharPos[ofs];
			i++;
		}
	}
	SDL_DestroyTexture(font_texture);
}

void PutString(SDL_Renderer *renderer, Uint16 x, Uint16 y, char *Text)
{
	SDL_Rect dest;
	Uint16 i;
	Uint8 CurrentChar;

	dest.y = y;
	dest.w = FontWidth / 2;   /* Set destination width to quarter size */
	dest.h = FontHeight / 2;  /* Set destination height to quarter size */

	for(i = 0; i < strlen(Text); i++)
	{
		CurrentChar = Text[i];
		dest.x = x + (i * (FontWidth / 2));  /* Adjust x position for quarter size */
		SDL_RenderCopy(renderer, Font, &CharPos[CurrentChar], &dest);
	}
}

int TextWidth2(SFont_FontInfo *Font, char *text)
{
	Uint16 ofs = 0;
	Uint16 i = 0, x = 0;

	while (text[i] != '\0')
	{
		if (text[i] == ' ')
		{
			x += Font->CharPos[2] - Font->CharPos[1];
			i++;
		}
		else
		{
			ofs = ((unsigned char) text[i] - 33) * 2 + 1;
			x += Font->CharPos[ofs + 1] - Font->CharPos[ofs];
			i++;
		}
	}
	return x;
}

int TextWidth(char *text)
{
	return (strlen(text) * FontWidth) / 4;  /* Return quarter width */
}

void XCenteredString2(SDL_Renderer *renderer, SFont_FontInfo *Font, int y,
		char *text)
{
	int w, h;
	SDL_GetRendererOutputSize(renderer, &w, &h);
	PutString2(renderer, Font, w / 2 - TextWidth2(Font, text) / 2, y,
			text);
}

void XCenteredString(SDL_Renderer *renderer, Uint16 y, char *Text)
{
	Uint16 x = (Graphics_GetScreenWidth() - TextWidth(Text)) / 2;
	PutString(renderer, x, y, Text);
}

void SFont_Input(SDL_Renderer *renderer, int x, int y, int Width, char *text)
{
	SDL_Event event;
	char ch;
	int done = 0;
	size_t text_len = strlen(text);

	while(!done) {
		if(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_KEYDOWN:
					switch(event.key.keysym.sym) {
						case SDLK_RETURN:
							done = 1;
							break;
						case SDLK_BACKSPACE:
							if(text_len > 0) {
								text[text_len - 1] = '\0';
								text_len--;
								PutString(renderer, x, y, text);
							}
							break;
						default:
							if(event.key.keysym.sym >= 32 && event.key.keysym.sym <= 126) {
								if(text_len < Width - 1) {
									ch = (char)event.key.keysym.sym;
									text[text_len] = ch;
									text[text_len + 1] = '\0';
									text_len++;
									PutString(renderer, x, y, text);
								}
							}
							break;
					}
					break;
				case SDL_QUIT:
					done = 1;
					break;
			}
		}
		SDL_RenderPresent(renderer);
	}
}
