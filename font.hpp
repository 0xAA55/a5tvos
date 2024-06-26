#pragma once

#include "graphics.hpp"

#include <cstdint>

namespace TVOS
{
	bool GetGlyphSize(uint32_t Unicode, int& Width, int& Height, bool Verbose);
	bool ExtractGlyph(ImageBlock& ImgOut, uint32_t Unicode, uint32_t color1, uint32_t color2, bool Verbose);
}
