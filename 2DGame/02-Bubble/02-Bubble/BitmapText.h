#ifndef _BITMAP_TEXT_INCLUDE
#define _BITMAP_TEXT_INCLUDE


#include <string>
#include "Texture.h"


// Builds a white-on-transparent RGBA texture with a 5x7 pixel bitmap font.
class BitmapText
{
public:
	static bool createTexture(Texture &tex, const std::string &text, int scale = 3);
};


#endif // _BITMAP_TEXT_INCLUDE
