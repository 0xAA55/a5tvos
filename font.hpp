#pragma once

#include "graphics.hpp"

#include <cstdint>

namespace TVOS
{
	void ExtractFont(ImageBlock& ImgOut, uint32_t Unicode, int& Width, int& Height, uint32_t color1, uint32_t color2);
}
