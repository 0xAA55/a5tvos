#include "font.hpp"

#include <unordered_map>
#include <cstddef>
#include <iostream>

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
			ret[AllGlyphsSet[i]] = int(i);
		}

		return ret;
	}

	static std::unordered_map<uint32_t, int> MakeCharToGlyphXPos()
	{
		std::unordered_map<uint32_t, int> ret;

		int xpos = 0;
		for(size_t i = 0; i < NumGlyphs; i++)
		{
			ret[AllGlyphsSet[i]] = xpos;
			xpos += GlyphWidthMap[i];
		}

		return ret;
	}

	static std::unordered_map<uint32_t, int> MakeCharToGlyphWidth()
	{
		std::unordered_map<uint32_t, int> ret;

		for (size_t i = 0; i < NumGlyphs; i++)
		{
			ret[AllGlyphsSet[i]] = GlyphWidthMap[i];
		}

		return ret;
	}

	static auto CharToGlyphMap = MakeCharToGlyphMap();
	static auto GlyphXPos = MakeCharToGlyphXPos();
	static auto GlyphWidth = MakeCharToGlyphWidth();

	static int GetGlyphPixel(int x, int y)
	{
		auto BytePtr = reinterpret_cast<const uint8_t*>(GlyphBinaryCode);
		int b = 7 - x % 8;
		int i = x / 8;
		int bit = 1 << b;
		return (BytePtr[y * BinaryCodeStride + i] & bit) ? 1 : 0;
	}

	bool GetGlyphSize(uint32_t Unicode, int& Width, int& Height, bool Verbose)
	{
		if (!CharToGlyphMap.count(Unicode))
		{
			if (Verbose)
			{
				std::cerr << "[WARN] In the call to `GetGlyphSize()`: Glyph U+" << std::hex << Unicode << std::dec << " not found.\n";
			}
			return false;
		}
		auto GlyphIndex = CharToGlyphMap.at(Unicode);
		Width = GlyphWidth.at(Unicode);
		Height = 22;
		return true;
	}

	bool ExtractGlyph(ImageBlock& ImgOut, uint32_t Unicode, uint32_t color1, uint32_t color2, bool Verbose)
	{
		if (!CharToGlyphMap.count(Unicode))
		{
			if (Verbose)
			{
				std::cerr << "[WARN] In the call to `ExtractGlyph()`: Glyph U+" << std::hex << Unicode << std::dec << " not found.\n";
			}
			return false;
		}
		auto GlyphIndex = CharToGlyphMap.at(Unicode);
		auto X = GlyphXPos.at(Unicode);
		ImgOut = ImageBlock();
		ImgOut.h = 22;
		ImgOut.w = GlyphWidth.at(Unicode);
		ImgOut.Pixels.resize(size_t(ImgOut.w) * ImgOut.h);
		for(int iy = 0 ; iy < ImgOut.h; iy ++)
		{
			for(int ix = 0; ix < ImgOut.w; ix ++)
			{
				ImgOut.Pixels[size_t(iy) * ImgOut.w + ix] = GetGlyphPixel(X + ix, iy) ? color1 : color2;
			}
		}
		return true;
	}
}
