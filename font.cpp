#include "font.hpp"

#include "graphics.hpp"

namespace TVOS
{
	uint16_t allchars[] =
	{
		#include "font/allchars"
	};
	uint32_t binarycode[]
	{
		#include "font/binarycode"
	};
	uint8_t widthtable[]
	{
		#include "font/widthtable"
	};
}
