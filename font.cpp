#include "font.hpp"

#include <unordered_map>
#include <cstddef>

namespace TVOS
{
	constexpr auto GlyphHeight = 22;
	constexpr auto NumGlyphs = 7541;
	static constexpr uint16_t AllGlyphsSet[NumGlyphs] =
	{
		#include "font/allglyphs"
	};

	static constexpr uint32_t GlyphBinaryCode[] =
	{
		#include "font/binarycode"
	};

	static constexpr uint8_t GlyphWidthMap[NumGlyphs] =
	{
		#include "font/widthtable"
	};

	static constexpr auto BinaryCodeStride = (sizeof GlyphBinaryCode) / GlyphHeight;

	static std::unordered_map<uint16_t, int> MakeCharToGlyphMap()
	{
		std::unordered_map<uint16_t, int> ret;

		for(size_t i = 0; i < NumGlyphs; i++)
		{
			ret[AllGlyphsSet[i]] = i;
		}

		return ret;
	}

	static std::unordered_map<uint16_t, int> MakeCharToGlyphXPos()
	{
		std::unordered_map<uint16_t, int> ret;

		int xpos = 0;
		for(size_t i = 0; i < NumGlyphs; i++)
		{
			ret[i] = xpos;
			xpos += GlyphWidthMap[i];
		}

		return ret;
	}

	static const auto CharToGlyphMap = MakeCharToGlyphMap();
	static const auto GlyphXPos = MakeCharToGlyphXPos();

	static int GetGlyphPixel(int x, int y)
	{
		int b = x % 32;
		int i = x / 32;
		int bit = 1 << b;
		return (GlyphBinaryCode[y * BinaryCodeStride / 4 + i] & bit) ? 1 : 0;
	}

	void GetGlyphSize(uint32_t Unicode, int& Width, int& Height)
	{
		auto GlyphIndex = CharToGlyphMap.at(Unicode);
		Width = GlyphWidthMap[GlyphIndex];
		Height = 22;
	}

	void ExtractGlyph(ImageBlock& ImgOut, uint32_t Unicode, uint32_t color1, uint32_t color2)
	{
		auto GlyphIndex = CharToGlyphMap.at(Unicode);
		ImgOut.h = 22;
		ImgOut.w = GlyphWidthMap[GlyphIndex];
		auto X = GlyphXPos.at(Unicode);
		ImgOut = ImageBlock();
		ImgOut.Pixels.resize(ImgOut.w * ImgOut.h);
		for(int iy = 0 ; iy < ImgOut.h; iy ++)
		{
			for(int ix = 0; ix < ImgOut.w; ix ++)
			{
				ImgOut.Pixels[iy * ImgOut.w + ix] = GetGlyphPixel(ix, iy) ? color1 : color2;
			}
		}
	}
}
