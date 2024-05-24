#include "utf.hpp"

namespace UTF
{
	UtfConversionError::UtfConversionError(const std::string& what) noexcept :
		std::runtime_error(what)
	{
	}

	std::vector<uint32_t> Utf8_to_Utf32(const std::string& Utf8s)
	{
		size_t i = 0;
		std::vector<uint32_t> ret;
		while(i < Utf8s.length())
		{
			if((Utf8s[i] & 0xFE) == 0xFC)//1111110x
			{
				if(i + 6 <= Utf8s.size())
				{
					ret.push_back
					(
						((uint32_t(Utf8s[i + 0]) & 0x01) << 30)|
						((uint32_t(Utf8s[i + 1]) & 0x3F) << 24)|
						((uint32_t(Utf8s[i + 2]) & 0x3F) << 18)|
						((uint32_t(Utf8s[i + 3]) & 0x3F) << 12)|
						((uint32_t(Utf8s[i + 4]) & 0x3F) << 6)|
						((uint32_t(Utf8s[i + 5]) & 0x3F) << 0)
					);
					i += 6;
				}
				else
					throw UtfConversionError(std::string("Need more bytes for decoding an UTF-8 string."));
			}
			else if((Utf8s[i] & 0xFC) == 0xF8)//111110xx
			{
				if(i + 5 <= Utf8s.size())
				{
					ret.push_back
					(
						((uint32_t(Utf8s[i + 0]) & 0x03) << 24)|
						((uint32_t(Utf8s[i + 1]) & 0x3F) << 18)|
						((uint32_t(Utf8s[i + 2]) & 0x3F) << 12)|
						((uint32_t(Utf8s[i + 3]) & 0x3F) << 6)|
						((uint32_t(Utf8s[i + 4]) & 0x3F) << 0)
					);
					i += 5;
				}
				else
					throw UtfConversionError(std::string("Need more bytes for decoding an UTF-8 string."));
			}
			else if((Utf8s[i] & 0xF8) == 0xF0)//11110xxx
			{
				if(i + 4 <= Utf8s.size())
				{
					ret.push_back
					(
						((uint32_t(Utf8s[i + 0]) & 0x07) << 18)|
						((uint32_t(Utf8s[i + 1]) & 0x3F) << 12)|
						((uint32_t(Utf8s[i + 2]) & 0x3F) << 6)|
						((uint32_t(Utf8s[i + 3]) & 0x3F) << 0)
					);
					i += 4;
				}
				else
					throw UtfConversionError(std::string("Need more bytes for decoding an UTF-8 string."));
			}
			else if((Utf8s[i] & 0xF0) == 0xE0)//1110xxxx
			{
				if(i + 3 <= Utf8s.size())
				{
					ret.push_back
					(
						((uint32_t(Utf8s[i + 0]) & 0x0F) << 12)|
						((uint32_t(Utf8s[i + 1]) & 0x3F) << 6)|
						((uint32_t(Utf8s[i + 2]) & 0x3F) << 0)
					);
					i += 3;
				}
				else
					throw UtfConversionError(std::string("Need more bytes for decoding an UTF-8 string."));
			}
			else if((Utf8s[i] & 0xE0) == 0xC0)//110xxxxx
			{
				if(i + 2 <= Utf8s.size())
				{
					ret.push_back
					(
						((uint32_t(Utf8s[i + 0]) & 0x1F) << 6)|
						((uint32_t(Utf8s[i + 1]) & 0x3F) << 0)
					);
					i += 2;
				}
				else
					throw UtfConversionError(std::string("Need more bytes for decoding an UTF-8 string."));
			}
			else if((Utf8s[i] & 0xC0) == 0x80)//10xxxxxx
			{
				throw UtfConversionError(std::string("Invalid start byte for UTF-8 string: ") + std::to_string(Utf8s[i]));
			}
			else if((Utf8s[i] & 0x80) == 0x00)//0xxxxxxx
			{
				ret.push_back(Utf8s[i] & 0x7F);
				i++;
			}
		}
		return ret;
	}

	std::string Utf32_to_Utf8(const std::vector<uint32_t>& Utf32s)
	{
		std::string ret;
		size_t i = 0;
		while(i < Utf32s.size())
		{
			if(Utf32s[i] >= 0x4000000)
			{
				ret.push_back(char(0xFC | ((Utf32s[i] >> 30) & 0x01)));
				ret.push_back(char(0x80 | ((Utf32s[i] >> 24) & 0x3F)));
				ret.push_back(char(0x80 | ((Utf32s[i] >> 18) & 0x3F)));
				ret.push_back(char(0x80 | ((Utf32s[i] >> 12) & 0x3F)));
				ret.push_back(char(0x80 | ((Utf32s[i] >> 6) & 0x3F)));
				ret.push_back(char(0x80 | (Utf32s[i] & 0x3F)));
				i += 6;
			}
			else if(Utf32s[i] >= 0x200000)
			{
				ret.push_back(char(0xF8 | ((Utf32s[i] >> 24) & 0x03)));
				ret.push_back(char(0x80 | ((Utf32s[i] >> 18) & 0x3F)));
				ret.push_back(char(0x80 | ((Utf32s[i] >> 12) & 0x3F)));
				ret.push_back(char(0x80 | ((Utf32s[i] >> 6) & 0x3F)));
				ret.push_back(char(0x80 | (Utf32s[i] & 0x3F)));
				i += 5;
			}
			else if(Utf32s[i] >= 0x10000)
			{
				ret.push_back(char(0xF0 | ((Utf32s[i] >> 18) & 0x07)));
				ret.push_back(char(0x80 | ((Utf32s[i] >> 12) & 0x3F)));
				ret.push_back(char(0x80 | ((Utf32s[i] >> 6) & 0x3F)));
				ret.push_back(char(0x80 | (Utf32s[i] & 0x3F)));
				i += 4;
			}
			else if(Utf32s[i] >= 0x0800)
			{
				ret.push_back(char(0xE0 | ((Utf32s[i] >> 12) & 0x0F)));
				ret.push_back(char(0x80 | ((Utf32s[i] >> 6) & 0x3F)));
				ret.push_back(char(0x80 | (Utf32s[i] & 0x3F)));
				i += 3;
			}
			else if(Utf32s[i] >= 0x0080)
			{
				ret.push_back(char(0xC0 | ((Utf32s[i] >> 6) & 0x1F)));
				ret.push_back(char(0x80 | (Utf32s[i] & 0x3F)));
				i += 2;
			}
			else
			{
				ret.push_back(char(Utf32s[i]));
				i++;
			}
		}
		return ret;
	}
}
