﻿#pragma once
#include <cstdint>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <memory>

namespace TVOS
{
	class OpenDeviceFailed: public std::runtime_error
	{
	public:
		OpenDeviceFailed(const std::string& what) noexcept;
	};
	
	uint32_t MakeColor(int cr, int cg, int cb);
	void GetColor(const uint32_t c, int& cr, int& cg, int& cb);

	struct ImageBlock
	{
		int w = 0;
		int h = 0;
		std::vector<uint32_t> Pixels;

		ImageBlock() = default;
		ImageBlock(const ImageBlock& ib) = default;
		ImageBlock(int width, int height);
		ImageBlock(int width, int height, uint32_t color);

		int GetStride() const;
		bool operator == (const ImageBlock& other) const;

		size_t GetSizeInBytes() const;
		ImageBlock& InvertPixelColors();
		ImageBlock& ReplacePixelColors(uint32_t find, uint32_t replace);
		uint32_t& GetPixel(int x, int y);
		uint32_t GetPixel(int x, int y) const;
		void PutPixel(int x, int y, uint32_t color);
	};

	class Graphics
	{
	public:
		Graphics(const std::string& fbdev);
		Graphics(const std::string& fbdev, bool Verbose);
		Graphics(const std::string& fbdev, int width, int height);
		Graphics(const std::string& fbdev, int width, int height, bool Verbose);
		Graphics();
		Graphics(bool Verbose);
		Graphics(int width, int height);
		Graphics(int width, int height, bool Verbose);
		Graphics(void* FBPtr);
		Graphics(void* FBPtr, bool Verbose);
		Graphics(void* FBPtr, int width, int height);
		Graphics(void* FBPtr, int width, int height, bool Verbose);

	protected:
		bool PreFitXYRB(int& x, int& y, int& r, int& b) const;
		bool GetWidthHeight(int x, int y, int r, int b, int& width, int& height) const;
		bool PreFitAreaGetWH(int& x, int& y, int& r, int& b, int& width, int& height) const;

		bool BackBufferMode = false;
		std::shared_ptr<ImageBlock> BackBuffer = nullptr;
		int BBReadPosX = 0;
		int BBReadPosY = 0;
		int BBWritePosX = 0;
		int BBWritePosY = 0;

		// 底层绘图操作
		void SetReadPos(int x, int y);
		void SetDrawPos(int x, int y);
		void WriteData(uint32_t color, int Repeat);
#if defined(_MSC_VER)
		virtual // 测试绘制前台的效果。
#endif
		void WriteData(const uint32_t* pixels, int Count);
		void WriteData(const std::vector<uint32_t>& pixels);
		void WriteData(int cr, int cg, int cb, int Repeat);
		std::vector<uint32_t> ReadPixelsRow(int x, int y, int count);
		
		void DrawImage(const ImageBlock& ib, int x, int y, int w, int h, int srcx, int srcy, int ops);
		void DrawImage(const ImageBlock& ib, int x, int y, int ops);

	public:
		int GetWidth() const;
		int GetHeight() const;

		void SetBackBufferMode(); // 绘制到后台缓冲区
		void SetFrontBufferMode(); // 绘制到前台fb
		bool IsBackBufferMode(); // 是否在绘制到后台缓冲区的模式里
		void RefreshFrontBuffer(); // 将后台缓冲区的内容刷新到前台缓冲区

		void ClearScreen(uint32_t color);

		ImageBlock ReadPixelsRect(int x, int y, int r, int b);
		ImageBlock ReadPixels(int x, int y, int w, int h);
		ImageBlock ReadPixels();

		void PutPixel(int x, int y, uint32_t color);
		void PutPixel(int x, int y, int cr, int cg, int cb);

		void DrawVLine(int x, int y1, int y2, uint32_t color);
		void DrawVLine(int x, int y1, int y2, int cr, int cg, int cb);
		void DrawVLineXor(int x, int y1, int y2);

		void DrawHLine(int x1, int x2, int y, uint32_t color);
		void DrawHLine(int x1, int x2, int y, int cr, int cg, int cb);
		void DrawHLineXor(int x1, int x2, int y);

		void DrawRect(int x, int y, int r, int b, uint32_t color);
		void DrawRect(int x, int y, int r, int b, int cr, int cg, int cb);
		void DrawRectXor(int x, int y, int r, int b);

		void FillRect(int x, int y, int r, int b, uint32_t color);
		void FillRect(int x, int y, int r, int b, int cr, int cg, int cb);
		void FillRectXor(int x, int y, int r, int b);
		void FillRectXor(int x, int y, int r, int b, uint32_t color);
		void FillRectAnd(int x, int y, int r, int b, uint32_t color);
		void FillRectOr(int x, int y, int r, int b, uint32_t color);

		void DrawImage(const ImageBlock& ib, int x, int y, int w, int h, int srcx, int srcy);
		void DrawImage(const ImageBlock& ib, int x, int y);

		void DrawImageAnd(const ImageBlock& ib, int x, int y, int w, int h, int srcx, int srcy);
		void DrawImageAnd(const ImageBlock& ib, int x, int y);

		void DrawImageOr(const ImageBlock& ib, int x, int y, int w, int h, int srcx, int srcy);
		void DrawImageOr(const ImageBlock& ib, int x, int y);

		void DrawImageXor(const ImageBlock& ib, int x, int y, int w, int h, int srcx, int srcy);
		void DrawImageXor(const ImageBlock& ib, int x, int y);

		void DrawText(int x, int y, const std::string& t, bool Transparent, uint32_t GlyphColor);
		void DrawTextXor(int x, int y, const std::string& t);
		void GetTextMetrics(const std::string& t, int& w, int& h) const;
		void GetTextMetrics(const std::string& t, int xlimit, int& w, int& h) const;

		const ImageBlock& GetBackBuffer() const;

	protected:
		std::string FBDev;
		std::fstream fs;
		int Width;
		int Height;
		int Stride;

		std::unordered_map<uint32_t, std::pair<ImageBlock, ImageBlock>> Glyphs;

		void GetGlyphMetrics(uint32_t GlyphUnicode, int& w, int& h) const;
		const ImageBlock& GetGlyph(uint32_t GlyphUnicode, bool InvertColor);
		void DrawGlyph(int x, int y, uint32_t GlyphUnicode, bool Transparent, uint32_t GlyphColor);
		void DrawGlyphXor(int x, int y, uint32_t GlyphUnicode);

		std::string ReadSimpleFile(const std::string& f);
		void GetFBSize(const std::string& fbdev, int& Width, int& Height);
		int GetFBStride(const std::string& fbdev);

	public:
		bool Verbose = false;
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

