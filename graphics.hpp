#pragma once
#include <cstdint>
#include <fstream>
#include <vector>
#include <unordered_map>

namespace TVOS
{
	uint32_t MakeColor(int cr, int cg, int cb);
	void GetColor(const uint32_t c, int& cr, int& cg, int& cb);

	struct ImageBlock
	{
		int w;
		int h;
		std::vector<uint32_t> Pixels;

		int GetStride() const;
		bool operator == (const ImageBlock& other) const;
	};

	class Graphics
	{
	public:
		Graphics(const std::string& fbdev);
		Graphics(const std::string& fbdev, int width, int height);
		Graphics();
		Graphics(int width, int height);

	protected:
		bool PreFitXYRB(int& x, int& y, int& r, int& b) const;
		bool GetWidthHeight(int x, int y, int r, int b, int& width, int& height) const;
		bool PreFitAreaGetWH(int& x, int& y, int& r, int& b, int& width, int& height) const;

		void SetReadPos(int x, int y);
		void SetDrawPos(int x, int y);
		void WriteData(uint32_t color, int Repeat);
		void WriteData(const uint32_t* pixels, int Count);
		void WriteData(const std::vector<uint32_t>& pixels);
		void WriteData(int cr, int cg, int cb, int Repeat);

		std::vector<uint32_t> ReadPixelsRow(int x, int y, int count);

	public:
		ImageBlock ReadPixels(int x, int y, int r, int b);

		void PutPixel(int x, int y, uint32_t color);
		void PutPixel(int x, int y, int cr, int cg, int cb);

		void FillRect(int x, int y, int r, int b, uint32_t color);
		void FillRect(int x, int y, int r, int b, int cr, int cg, int cb);

		void DrawImages(const ImageBlock& ib, int x, int y, int w, int h, int srcx, int srcy);

	public:
		void DrawText(int x, int y, const std::string& t);
		void GetTextMetrics(const std::string& t, int& w, int& h) const;

	protected:
		std::string FBDev;
		std::ifstream ifs;
		std::ofstream ofs;
		int Width;
		int Height;
		int Stride;

		std::unordered_map<uint32_t, ImageBlock> Glyphs;
		std::unordered_map<uint32_t, int> GlyphsUsage;
		size_t GlyphMapMemoryUsage = 0;

		void DrawGlyph(int x, int y, uint32_t Glyph);

		static std::string ReadFile(const std::string& f);
		static void GetFBSize(const std::string& fbdev, int& Width, int& Height);
		static int GetFBStride(const std::string& fbdev);
	};
}

namespace std
{
	template<> struct hash<vector<uint32_t>>
	{
		inline size_t operator () (const vector<uint32_t>& v)
		{
			size_t h = 0;
			for(size_t i = 0; i < v.size(); i++)
			{
				h = (h << 4) | (h >> ((sizeof h) * 8 - 4));
				h += size_t(v[i]);
			}
			return h;
		}
	};
	template<> struct hash<TVOS::ImageBlock>
	{
		inline size_t operator () (const TVOS::ImageBlock& b)
		{
			auto VectorHasher = hash<vector<uint32_t>>();
			return
				(size_t(b.w) << 0) +
				(size_t(b.h) << 4) +
				(VectorHasher(b.Pixels) << 8);
		}
	};
}

#include "font.hpp"
