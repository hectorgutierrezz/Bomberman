#include "BitmapText.h"
#include <cctype>
#include <vector>


// 5x7 glyphs, one byte per column (bit 0 = top row)
static const unsigned char FONT_SPACE[5] = {0, 0, 0, 0, 0};

static const unsigned char FONT_A[5] = {0x3E, 0x09, 0x09, 0x09, 0x3E};
static const unsigned char FONT_B[5] = {0x3F, 0x25, 0x25, 0x25, 0x1A};
static const unsigned char FONT_C[5] = {0x1E, 0x21, 0x21, 0x21, 0x12};
static const unsigned char FONT_D[5] = {0x3F, 0x21, 0x21, 0x21, 0x1E};
static const unsigned char FONT_E[5] = {0x3F, 0x25, 0x25, 0x25, 0x21};
static const unsigned char FONT_F[5] = {0x3F, 0x05, 0x05, 0x05, 0x01};
static const unsigned char FONT_G[5] = {0x1E, 0x21, 0x25, 0x25, 0x3A};
static const unsigned char FONT_H[5] = {0x3F, 0x04, 0x04, 0x04, 0x3F};
static const unsigned char FONT_I[5] = {0x21, 0x21, 0x3F, 0x21, 0x21};
static const unsigned char FONT_J[5] = {0x10, 0x20, 0x20, 0x20, 0x1F};
static const unsigned char FONT_K[5] = {0x3F, 0x04, 0x0A, 0x11, 0x20};
static const unsigned char FONT_L[5] = {0x3F, 0x20, 0x20, 0x20, 0x20};
static const unsigned char FONT_M[5] = {0x3F, 0x02, 0x04, 0x02, 0x3F};
static const unsigned char FONT_N[5] = {0x3F, 0x02, 0x04, 0x08, 0x3F};
static const unsigned char FONT_O[5] = {0x1E, 0x21, 0x21, 0x21, 0x1E};
static const unsigned char FONT_P[5] = {0x3F, 0x09, 0x09, 0x09, 0x06};
static const unsigned char FONT_Q[5] = {0x1E, 0x21, 0x29, 0x11, 0x2E};
static const unsigned char FONT_R[5] = {0x3F, 0x09, 0x09, 0x19, 0x26};
static const unsigned char FONT_S[5] = {0x12, 0x25, 0x25, 0x25, 0x1A};
static const unsigned char FONT_T[5] = {0x01, 0x01, 0x3F, 0x01, 0x01};
static const unsigned char FONT_U[5] = {0x1F, 0x20, 0x20, 0x20, 0x1F};
static const unsigned char FONT_V[5] = {0x0F, 0x10, 0x20, 0x10, 0x0F};
static const unsigned char FONT_W[5] = {0x3F, 0x10, 0x08, 0x10, 0x3F};
static const unsigned char FONT_X[5] = {0x31, 0x0A, 0x04, 0x0A, 0x31};
static const unsigned char FONT_Y[5] = {0x03, 0x04, 0x38, 0x04, 0x03};
static const unsigned char FONT_Z[5] = {0x31, 0x29, 0x25, 0x23, 0x21};

static const unsigned char FONT_0[5] = {0x3E, 0x45, 0x49, 0x51, 0x3E};
static const unsigned char FONT_1[5] = {0x00, 0x42, 0x7F, 0x40, 0x00};
static const unsigned char FONT_2[5] = {0x42, 0x61, 0x51, 0x49, 0x46};
static const unsigned char FONT_3[5] = {0x21, 0x41, 0x45, 0x4B, 0x31};
static const unsigned char FONT_4[5] = {0x18, 0x14, 0x12, 0x7F, 0x10};
static const unsigned char FONT_5[5] = {0x27, 0x45, 0x45, 0x45, 0x39};
static const unsigned char FONT_6[5] = {0x3C, 0x4A, 0x49, 0x49, 0x30};
static const unsigned char FONT_7[5] = {0x01, 0x71, 0x09, 0x05, 0x03};
static const unsigned char FONT_8[5] = {0x36, 0x49, 0x49, 0x49, 0x36};
static const unsigned char FONT_9[5] = {0x06, 0x49, 0x49, 0x29, 0x1E};
static const unsigned char FONT_COLON[5] = {0x00, 0x36, 0x36, 0x00, 0x00};

static const unsigned char *glyphFor(char c)
{
	switch(c)
	{
	case '0': return FONT_0;
	case '1': return FONT_1;
	case '2': return FONT_2;
	case '3': return FONT_3;
	case '4': return FONT_4;
	case '5': return FONT_5;
	case '6': return FONT_6;
	case '7': return FONT_7;
	case '8': return FONT_8;
	case '9': return FONT_9;
	case ':': return FONT_COLON;
	case 'A': return FONT_A;
	case 'B': return FONT_B;
	case 'C': return FONT_C;
	case 'D': return FONT_D;
	case 'E': return FONT_E;
	case 'F': return FONT_F;
	case 'G': return FONT_G;
	case 'H': return FONT_H;
	case 'I': return FONT_I;
	case 'J': return FONT_J;
	case 'K': return FONT_K;
	case 'L': return FONT_L;
	case 'M': return FONT_M;
	case 'N': return FONT_N;
	case 'O': return FONT_O;
	case 'P': return FONT_P;
	case 'Q': return FONT_Q;
	case 'R': return FONT_R;
	case 'S': return FONT_S;
	case 'T': return FONT_T;
	case 'U': return FONT_U;
	case 'V': return FONT_V;
	case 'W': return FONT_W;
	case 'X': return FONT_X;
	case 'Y': return FONT_Y;
	case 'Z': return FONT_Z;
	default:  return FONT_SPACE;
	}
}

bool BitmapText::createTexture(Texture &tex, const std::string &text, int scale)
{
	const int charW = 5;
	const int charH = 7;
	const int gap = 1;

	if(text.empty())
		return false;

	int width = int(text.size()) * (charW + gap) * scale - gap * scale;
	int height = charH * scale;
	if(width <= 0)
		return false;

	std::vector<unsigned char> pixels(width * height * 4, 0);

	for(size_t i = 0; i < text.size(); ++i)
	{
		char c = char(toupper(static_cast<unsigned char>(text[i])));
		const unsigned char *glyph = glyphFor(c);
		int baseX = int(i) * (charW + gap) * scale;

		for(int col = 0; col < charW; ++col)
		{
			unsigned char bits = glyph[col];
			for(int row = 0; row < charH; ++row)
			{
				if((bits & (1 << row)) == 0)
					continue;

				for(int sy = 0; sy < scale; ++sy)
				{
					for(int sx = 0; sx < scale; ++sx)
					{
						int x = baseX + col * scale + sx;
						int y = row * scale + sy;
						int idx = (y * width + x) * 4;
						pixels[idx + 0] = 255;
						pixels[idx + 1] = 255;
						pixels[idx + 2] = 255;
						pixels[idx + 3] = 255;
					}
				}
			}
		}
	}

	tex.loadFromRGBABuffer(pixels.data(), width, height);
	tex.setMinFilter(GL_NEAREST);
	tex.setMagFilter(GL_NEAREST);
	tex.setWrapS(GL_CLAMP_TO_EDGE);
	tex.setWrapT(GL_CLAMP_TO_EDGE);
	return true;
}
