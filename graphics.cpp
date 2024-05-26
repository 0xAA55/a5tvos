#include "graphics.hpp"
#include "font.hpp"
#include "utf.hpp"

#include <cstring>
#include <iostream>

namespace TVOS
{
	OpenDeviceFailed::OpenDeviceFailed(const std::string& what) noexcept :
		std::runtime_error(what)
	{
	}
	uint32_t MakeColor(int cr, int cg, int cb)
	{
		return
			(uint32_t(cr > 255 ? 255 : cr < 0 ? 0 : cr) << 16) |
			(uint32_t(cg > 255 ? 255 : cg < 0 ? 0 : cg) <<  8) |
			(uint32_t(cb > 255 ? 255 : cb < 0 ? 0 : cb) <<  0) |
			0xFF000000;
	}

	void GetColor(const uint32_t c, int& cr, int& cg, int& cb)
	{
		cr = int(c & 0x00FF0000) >> 16;
		cg = int(c & 0x0000FF00) >>  8;
		cb = int(c & 0x000000FF) >>  0;
	}

	ImageBlock::ImageBlock(int width, int height) :
		w(width),
		h(height)
	{
		Pixels.resize(w * h);
	}

	int ImageBlock::GetStride() const
	{
		return w * 4;
	}

	bool ImageBlock::operator == (const ImageBlock& other) const
	{
		return
			w == other.w &&
			h == other.h &&
			Pixels == other.Pixels;
	}

	size_t ImageBlock::GetSizeInBytes() const
	{
		return (sizeof *this) + Pixels.size() * (sizeof Pixels[0]);
	}

	ImageBlock& ImageBlock::InvertPixelColors()
	{
		for(size_t i = 0; i < Pixels.size(); i++)
		{
			Pixels[i] ^= 0xFFFFFF;
		}
		return *this;
	}

	ImageBlock& ImageBlock::ReplacePixelColors(uint32_t find, uint32_t replace)
	{
		for(size_t i = 0; i < Pixels.size(); i++)
		{
			if (Pixels[i] == find)
			{
				Pixels[i] = replace;
			}
		}
		return *this;
	}

	bool Graphics::PreFitXYRB(int& x, int& y, int& r, int& b) const
	{
		if (x > r) {int t = r; r = x; x = t;}
		if (y > b) {int t = b; b = y; y = t;}
		if (x < 0) x = 0;
		if (y < 0) y = 0;
		if (x >= Width || y >= Height) return false;
		if (r < 0 || b < 0) return false;
		return true;
	}
	
	bool Graphics::GetWidthHeight(int x, int y, int r, int b, int& width, int& height) const
	{
		if (!PreFitXYRB(x, y, r, b)) return false;

		width = r + 1 - x;
		height = b + 1 - y;
		return true;
	}

	bool Graphics::PreFitAreaGetWH(int& x, int& y, int& r, int& b, int& width, int& height) const
	{
		if (!PreFitXYRB(x, y, r, b)) return false;

		width = r + 1 - x;
		height = b + 1 - y;
		return true;
	}

	ImageBlock Graphics::ReadPixelsRect(int x, int y, int r, int b)
	{
		ImageBlock ret;
		int width, height;
		if (!PreFitAreaGetWH(x, y, r, b, width, height)) return ret;
		ret.w = width;
		ret.h = height;

		if (Verbose)
		{
			std::cout << "[INFO] Reading rectangle pixels: x=" << x << ", y=" << y << ", r=" << r << ", b=" << b << ", w=" << width << ", h=" << height << ".\n";
		}

		for(int i = 0; i < height; i++)
		{
			auto row = ReadPixelsRow(x, y + i, width);
			ret.Pixels.insert(ret.Pixels.end(), row.cbegin(), row.cend());
		}
		return ret;
	}

	ImageBlock Graphics::ReadPixels(int x, int y, int w, int h)
	{
		return ReadPixelsRect(x, y, x + w - 1, y + h - 1);
	}

	Graphics::Graphics(const std::string& fbdev) :
		Graphics(fbdev, Verbose)
	{
	}

	Graphics::Graphics(const std::string& fbdev, bool Verbose):
		fs(std::fstream(std::string("/dev/") + fbdev, std::ios::binary | std::ios::in | std::ios::out)),
		Verbose(Verbose)
	{
		if (Verbose)
		{
			std::cout << "[INFO] Opening `/dev/" << fbdev << "` in binary input/output mode.\n";
		}
		try
		{
			fs.exceptions(std::ios::badbit | std::ios::failbit);
		} catch (const std::ios::failure& e)
		{
			throw OpenDeviceFailed(std::string("Open device `/dev/") + fbdev + "` failed: " + e.what());
		}

		GetFBSize(fbdev, Width, Height);
		Stride = GetFBStride(fbdev);
		if (Verbose)
		{
			std::cout << "[INFO] Resolution of `/dev/" << fbdev << "` is " << Width << "x" << Height << ", with stride = " << Stride << ".\n";
		}
	}
	
	Graphics::Graphics(const std::string& fbdev, int width, int height) :
		Graphics(fbdev, width, height, Verbose)
	{
	}

	Graphics::Graphics(const std::string& fbdev, int width, int height, bool Verbose) :
		Graphics(fbdev, Verbose)
	{
		Width = width;
		Height = height;
		Stride = GetFBStride(fbdev);

		if (Verbose)
		{
			std::cout << "[INFO] Changed the resolution of `/dev/" << fbdev << "` to " << Width << "x" << Height << ".\n";
		}
	}

	Graphics::Graphics() : Graphics("fb0")
	{
	}

	Graphics::Graphics(bool Verbose) : Graphics("fb0", Verbose)
	{
	}

	Graphics::Graphics(int width, int height) : Graphics("fb0", width, height)
	{
	}

	Graphics::Graphics(int width, int height, bool Verbose) : Graphics("fb0", width, height, Verbose)
	{
	}

	Graphics::Graphics(void* FBPtr) :
		Graphics(FBPtr, 800, 480, Verbose)
	{
	}

	Graphics::Graphics(void* FBPtr, bool Verbose) :
		Graphics(FBPtr, 800, 480, Verbose)
	{
	}

	Graphics::Graphics(void* FBPtr, int width, int height) :
		Graphics(FBPtr, width, height, Verbose)
	{
	}

	Graphics::Graphics(void* FBPtr, int width, int height, bool Verbose) :
		Width(width),
		Height(height),
		Stride(width * 4),
		Verbose(Verbose),
		BackBufferMode(true),
		BackBuffer(std::make_shared<ImageBlock>(width, height))
	{
		if (FBPtr) memcpy(&BackBuffer->Pixels[0], FBPtr, Stride * Height);
	}

	std::string Graphics::ReadSimpleFile(const std::string& f)
	{
		if (Verbose)
		{
			std::cout << "[INFO] Reading `" << f << "`.\n";
		}
		std::ifstream r(f);
		r.exceptions(std::ios::badbit | std::ios::failbit);
		std::string ret;
		std::getline(r, ret);
		if (Verbose)
		{
			std::cout << "[INFO] Got: `" << ret << "`.\n";
		}
		return ret;
	}

	void Graphics::GetFBSize(const std::string& fbdev, int& Width, int& Height)
	{
		Width = Height = 0;
		auto StringSize = ReadSimpleFile(std::string("/sys/class/graphics/") + fbdev + "/virtual_size");
		if (StringSize.length())
		{
			StringSize.push_back('\0');
			char* ch = strchr(&StringSize[0], ',');
			Width = std::stoi(&StringSize[0]);
			Height = std::stoi(ch + 1);
		}
	}

	int Graphics::GetFBStride(const std::string& fbdev)
	{
		return std::stoi(ReadSimpleFile(std::string("/sys/class/graphics/") + fbdev + "/stride"));
	}

	void Graphics::SetDrawPos(int x, int y)
	{
		if (!BackBufferMode)
		{
			if (Verbose)
			{
				std::cout << "[INFO] Set draw position: " << x << ", " << y << ".\n";
			}
			int Row = int(y) * Stride;
			fs.seekp(Row + x * 4, std::ios::beg);
		}
		BBWritePosX = x;
		BBWritePosY = y;
	}

	void Graphics::SetReadPos(int x, int y)
	{
		if (!BackBufferMode)
		{
			if (Verbose)
			{
				std::cout << "[INFO] Set read position: " << x << ", " << y << ".\n";
			}
			int Row = int(y) * Stride;
			fs.seekg(Row + x * 4, std::ios::beg);
		}
		BBReadPosX = x;
		BBReadPosY = y;
	}

	void Graphics::WriteData(uint32_t color, int Repeat)
	{
		std::vector<uint32_t> Pixels;
		Pixels.resize(Repeat);
		for(int i = 0; i < Repeat; i++) Pixels[i] = color;
		WriteData(Pixels);
	}

	void Graphics::WriteData(const uint32_t* pixels, int Count)
	{
		// if (Verbose)
		// {
		// 	std::cout << "[INFO] Writting " << Count << " pixels from pointer " << std::hex << pixels << std::dec << ".\n";
		// }
		if (BackBufferMode)
		{
			auto* Buffer = BackBuffer.get();
			memcpy(&Buffer->Pixels[size_t(BBWritePosY) * Buffer->w + BBWritePosX], pixels, size_t(Count) * 4);
		}
		else
		{
			fs.write(reinterpret_cast<const char*>(pixels), size_t(Count) * 4);
		}
	}
	
	void Graphics::WriteData(const std::vector<uint32_t>& pixels)
	{
		WriteData(&pixels[0], int(pixels.size()));
	}
	
	void Graphics::WriteData(int cr, int cg, int cb, int Repeat)
	{
		WriteData(MakeColor(cr, cg, cb), Repeat);
	}

	void Graphics::PutPixel(int x, int y, uint32_t color)
	{
		SetDrawPos(x, y);
		WriteData(color, 1);
	}
	
	void Graphics::PutPixel(int x, int y, int cr, int cg, int cb)
	{
		PutPixel(x, y, MakeColor(cr, cg, cb));
	}
	
	std::vector<uint32_t> Graphics::ReadPixelsRow(int x, int y, int count)
	{
		std::vector<uint32_t> ret;
		if (x + count > Width) count = Width - x;
		if (count <= 0) return ret;
		SetReadPos(x, y);
		if (BackBufferMode)
		{
			auto Buffer = BackBuffer.get();
			ret.resize(count);
			memcpy(&ret[0], &Buffer->Pixels[BBReadPosY * Buffer->w + BBReadPosX], size_t(count) * 4);
		}
		else
		{
			ret.resize(count);
			fs.read(reinterpret_cast<char*>(&ret[0]), size_t(count) * 4);
		}
		return ret;
	}

	int Graphics::GetWidth() const
	{
		return Width;
	}

	int Graphics::GetHeight() const
	{
		return Height;
	}

	void Graphics::SetBackBufferMode()
	{
		BackBufferMode = true;
		if (!BackBuffer)
		{
			BackBuffer = std::make_shared<ImageBlock>(Width, Height);
			ClearScreen(0xFFFFFFFF);
		}
	}

	void Graphics::SetFrontBufferMode()
	{
		BackBufferMode = false;
	}
	
	bool Graphics::IsBackBufferMode()
	{
		return BackBufferMode;
	}
	
	void Graphics::RefreshFrontBuffer()
	{
		DrawImage(*BackBuffer, 0, 0);
	}

	void Graphics::DrawVLine(int x, int y1, int y2, uint32_t color)
	{
		FillRect(x, y1, x, y2, color);
	}

	void Graphics::DrawVLine(int x, int y1, int y2, int cr, int cg, int cb)
	{
		DrawVLine(x, y1, y2, MakeColor(cr, cg, cb));
	}
	
	void Graphics::DrawVLineXor(int x, int y1, int y2)
	{
		FillRectXor(x, y1, x, y2);
	}

	void Graphics::DrawHLine(int x1, int x2, int y, uint32_t color)
	{
		FillRect(x1, y, x2, y, color);
	}

	void Graphics::DrawHLine(int x1, int x2, int y, int cr, int cg, int cb)
	{
		DrawHLine(x1, x2, y, MakeColor(cr, cg, cb));
	}
	
	void Graphics::DrawHLineXor(int x1, int x2, int y)
	{
		FillRectXor(x1, y, x2, y);
	}

	void Graphics::DrawRect(int x, int y, int r, int b, uint32_t color)
	{
		if (!PreFitXYRB(x, y, r, b)) return;
		DrawHLine(x, r, y, color);
		DrawHLine(x, r, b, color);
		if (b - y < 2) return;
		DrawVLine(x, y + 1, b - 1, color);
		DrawVLine(r, y + 1, b - 1, color);
	}
	
	void Graphics::DrawRectXor(int x, int y, int r, int b)
	{
		if (!PreFitXYRB(x, y, r, b)) return;
		DrawHLineXor(x, r, y);
		DrawHLineXor(x, r, b);
		if (b - y < 2) return;
		DrawVLineXor(x, y + 1, b - 1);
		DrawVLineXor(r, y + 1, b - 1);
	}

	void Graphics::DrawRect(int x, int y, int r, int b, int cr, int cg, int cb)
	{
		DrawRect(x, y, r, b, MakeColor(cr, cg, cb));
	}

	void Graphics::FillRect(int x, int y, int r, int b, uint32_t color)
	{
		int w, h;
		if (!PreFitAreaGetWH(x, y, r, b, w, h)) return;

		for(int iy = y; iy <= b; iy ++)
		{
			SetDrawPos(x, iy);
			WriteData(color, w);
		}
	}

	void Graphics::FillRect(int x, int y, int r, int b, int cr, int cg, int cb)
	{
		FillRect(x, y, r, b, MakeColor(cr, cg, cb));
	}

	void Graphics::FillRectXor(int x, int y, int r, int b)
	{
		if (!PreFitXYRB(x, y, r, b)) return;

		auto ImageSrc = ReadPixelsRect(x, y, r, b).InvertPixelColors();
		DrawImage(ImageSrc, x, y);
	}

	void Graphics::DrawImage(const ImageBlock& ib, int x, int y, int w, int h, int srcx, int srcy, int ops)
	{
		if (ops == 0) { DrawImage(ib, x, y, w, h, srcx, srcy); return; }

		if (Verbose)
		{
			std::cout << "[INFO] Drawing image 0x" << std::hex << size_t(&ib) << std::dec << " at x=" << x << ", y=" << y << ", w=" << w << ", h=" << h << ", srcx=" << srcx << ", srcy=" << srcy << ", ops=" << ops << ".\n";
		}

		auto ImageSrc = ReadPixels(x, y, w, h);
		for(int y = 0; y < ImageSrc.h; y++)
		{
			if (y >= ib.h) break;
			for(int x = 0; x < ImageSrc.w; x++)
			{
				if (x >= ib.w) break;
				switch(ops)
				{
				case 1:  ImageSrc.Pixels[y * ImageSrc.w + x] &= ib.Pixels[y * ib.w + x]; break;
				case 2:  ImageSrc.Pixels[y * ImageSrc.w + x] |= ib.Pixels[y * ib.w + x]; break;
				case 3:  ImageSrc.Pixels[y * ImageSrc.w + x] ^= ib.Pixels[y * ib.w + x]; break;
				}
			}
		}
		DrawImage(ImageSrc, x, y, w, h, srcx, srcy);
	}

	void Graphics::DrawImage(const ImageBlock& ib, int x, int y, int ops)
	{
		DrawImage(ib, x, y, ib.w, ib.h, 0, 0, ops);
	}

	void Graphics::DrawImage(const ImageBlock& ib, int x, int y, int w, int h, int srcx, int srcy)
	{
		if (Verbose)
		{
			std::cout << "[INFO] Drawing image 0x" << std::hex << size_t(&ib) << std::dec << " at x=" << x << ", y=" << y << ", w=" << w << ", h=" << h << ", srcx=" << srcx << ", srcy=" << srcy << ".\n";
		}

		if (x < 0)
		{
			srcx -= x;
			w += x;
			x = 0;
		}
		if (y < 0)
		{
			srcy -= y;
			h += y;
			y = 0;
		}
		if (srcx > ib.w || srcy > ib.h) return;
		if (x + w > Width) w = Width - x;
		if (y + h > Height) h = Height - y;
		if (w <= 0 || h <= 0) return;
		int srcw = ib.w - srcx;
		int srch = ib.h - srcy;
		w = w > srcw ? srcw : w;
		h = h > srch ? srch : h;

		for(int iy = 0; iy < h; iy ++)
		{
			SetDrawPos(x, iy + y);
			WriteData(&ib.Pixels[(iy + srcy) * ib.w + srcx], w);
		}
	}

	void Graphics::DrawImage(const ImageBlock& ib, int x, int y)
	{
		DrawImage(ib, x, y, ib.w, ib.h, 0, 0);
	}

	void Graphics::DrawImageAnd(const ImageBlock& ib, int x, int y, int w, int h, int srcx, int srcy)
	{
		DrawImage(ib, x, y, w, h, srcx, srcy, 1);
	}

	void Graphics::DrawImageAnd(const ImageBlock& ib, int x, int y)
	{
		DrawImage(ib, x, y, 1);
	}
	
	void Graphics::DrawImageOr(const ImageBlock& ib, int x, int y, int w, int h, int srcx, int srcy)
	{
		DrawImage(ib, x, y, w, h, srcx, srcy, 2);
	}
	
	void Graphics::DrawImageOr(const ImageBlock& ib, int x, int y)
	{
		DrawImage(ib, x, y, 2);
	}
	
	void Graphics::DrawImageXor(const ImageBlock& ib, int x, int y, int w, int h, int srcx, int srcy)
	{
		DrawImage(ib, x, y, w, h, srcx, srcy, 3);
	}
	
	void Graphics::DrawImageXor(const ImageBlock& ib, int x, int y)
	{
		DrawImage(ib, x, y, 3);
	}

	void Graphics::ClearScreen(uint32_t color)
	{
		FillRect(0, 0, Width - 1, Height - 1, color);
	}

	void Graphics::GetGlyphMetrics(uint32_t GlyphUnicode, int& w, int& h) const
	{ // 不能获取到字符大小的时候获取问号的字符大小
		if (GetGlyphSize(GlyphUnicode, w, h, Verbose)) return;
		GetGlyphSize('?', w, h, Verbose);
	}

	const ImageBlock& Graphics::GetGlyph(uint32_t GlyphUnicode)
	{
		if (Glyphs.count(GlyphUnicode))
		{
			if (Verbose)
			{
				std::cout << "[INFO] Retrieving cached glyph U+" << std::hex << GlyphUnicode << std::dec << ".\n";
			}
			return Glyphs.at(GlyphUnicode);
		}
		else
		{
			if (Verbose)
			{
				std::cout << "[INFO] Creating glyph cache U+" << std::hex << GlyphUnicode << std::dec << ".\n";
			}

			// 生成字体
			if (!ExtractGlyph(Glyphs[GlyphUnicode], GlyphUnicode, 0xFF000000, 0xFFFFFFFF, Verbose))
			{ // 不能显示的字符使用问号
				if (Verbose)
				{
					std::cout << "[INFO] Create glyph cache U+" << std::hex << GlyphUnicode << std::dec << " failed.\n";
				}
				return GetGlyph('?');
			}

			auto& GlyphImage = Glyphs[GlyphUnicode];
			if (Verbose)
			{
				std::cout << "[INFO] Glyph cache U+" << std::hex << GlyphUnicode << std::dec << " has w=" << GlyphImage.w << ", h=" << GlyphImage.h << ".\n";
			}
			return GlyphImage;
		}
	}

	void Graphics::DrawGlyph(int x, int y, uint32_t GlyphUnicode, bool Transparent, uint32_t GlyphColor)
	{
		if (Verbose)
		{
			std::cout << "[INFO] Drawing a glyph U+" << std::hex << GlyphUnicode << std::dec << " at x=" << x << ", y=" << y << " with `Transparent=" << (Transparent ? "true" : "false") << "`.\n";
		}
		auto& GlyphImage = GetGlyph(GlyphUnicode);
		if (!Transparent && GlyphColor == 0)
		{
			DrawImage(GlyphImage, x, y);
		}
		else
		{
			if (Transparent)
			{
				DrawImageAnd(GlyphImage, x, y);
			}
			if (GlyphColor != 0)
			{
				ImageBlock NewGlyphImage = GlyphImage;
				NewGlyphImage.InvertPixelColors();
				if (GlyphColor != 0xFFFFFFFF) NewGlyphImage.ReplacePixelColors(0xFFFFFFFF, GlyphColor);
				DrawImageOr(NewGlyphImage, x, y);
			}
		}
	}

	void Graphics::DrawGlyphXor(int x, int y, uint32_t GlyphUnicode)
	{
		if (Verbose)
		{
			std::cout << "[INFO] Drawing a glyph U+" << std::hex << GlyphUnicode << std::dec << " at x=" << x << ", y=" << y << " with `XOR` opcode.\n";
		}
		ImageBlock GlyphImage = GetGlyph(GlyphUnicode);
		GlyphImage.InvertPixelColors();
		DrawImageXor(GlyphImage, x, y);
	}

	void Graphics::GetTextMetrics(const std::string& t, int& w, int& h) const
	{
		w = 0;
		h = 0;

		for(auto& ch: UTF::Utf8_to_Utf32(t))
		{
			int w_, h_;
			GetGlyphMetrics(ch, w_, h_);
			w += w_;
			h = h < h_ ? h_ : h;
		}
	}

	const ImageBlock& Graphics::GetBackBuffer() const
	{
		return *BackBuffer;
	}

	void Graphics::DrawText(int x, int y, const std::string& t, bool Transparent, uint32_t GlyphColor)
	{
		for(auto& ch: UTF::Utf8_to_Utf32(t))
		{
			int w, h;
			GetGlyphMetrics(ch, w, h);
			DrawGlyph(x, y, ch, Transparent, GlyphColor);
			x += w;
		}
	}
	void Graphics::DrawTextXor(int x, int y, const std::string& t)
	{
		for (auto& ch : UTF::Utf8_to_Utf32(t))
		{
			int w, h;
			GetGlyphMetrics(ch, w, h);
			DrawGlyphXor(x, y, ch);
			x += w;
		}
	}
}
