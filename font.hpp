#pragma once

#include "graphics.hpp"

#include <cstdint>

namespace TVOS
{
	void GetGlyphSize(uint32_t Unicode, int& Width, int& Height);
	void ExtractGlyph(ImageBlock& ImgOut, uint32_t Unicode, int& Width, int& Height, uint32_t color1, uint32_t color2);
}
