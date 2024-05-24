#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <cstddef>
#include <stdexcept>

namespace UTF
{
	class UtfConversionError : public std::runtime_error
	{
	public:
		UtfConversionError(const std::string& what) noexcept;
	};

	std::vector<uint32_t> Utf8_to_Utf32(const std::string& Utf8s);
	std::string Utf32_to_Utf8(const std::vector<uint32_t>& Utf32s);
}

