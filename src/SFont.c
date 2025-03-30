#include <SDL.h>
#include <stdlib.h>
#include <string.h>

#include "SFont.h"
#include "graphics.h"

static Uint32 GetPixel(SDL_Surface *, Sint32, Sint32);
static SFont_FontInfo InternalFont;
static SDL_Texture *Font = NULL;
static int FontH;
static Uint8 *FontMap;
static SDL_Rect FontRect;

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

void InitFont(SDL_Texture *font)
{
	int x = 0;
	int width;
	SDL_QueryTexture(font, NULL, NULL, &width, &FontH);
	FontMap = (Uint8*)malloc(sizeof(Uint8) * width);

	Font = font;
	FontRect.h = FontH;
	FontRect.y = 0;
	FontRect.x = 0;
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

void PutString(SDL_Renderer *renderer, int x, int y, char *text)
{
	if(text == NULL)
		return;

	int i;
	for(i = 0; text[i] != '\0' && x <= Graphics_GetScreenWidth(); i++) {
		FontRect.x = FontMap[text[i]];
		FontRect.w = FontMap[text[i] + 1] - FontRect.x;
		SDL_Rect dest = {x, y, FontRect.w, FontH};
		SDL_RenderCopy(renderer, Font, &FontRect, &dest);
		x += FontRect.w;
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
	int i;
	int width = 0;

	if(text == NULL)
		return 0;

	for(i = 0; text[i] != '\0'; i++)
		width += FontMap[text[i] + 1] - FontMap[text[i]];

	return width;
}

void XCenteredString2(SDL_Renderer *renderer, SFont_FontInfo *Font, int y,
		char *text)
{
	int w, h;
	SDL_GetRendererOutputSize(renderer, &w, &h);
	PutString2(renderer, Font, w / 2 - TextWidth2(Font, text) / 2, y,
			text);
}

void XCenteredString(SDL_Renderer *renderer, int y, char *text)
{
	PutString(renderer, (Graphics_GetScreenWidth() - TextWidth(text)) / 2, y, text);
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
