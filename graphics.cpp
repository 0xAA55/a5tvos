#include "graphics.hpp"
#include "font.hpp"

#include <cstring>
#include <iostream>

namespace TVOS
{
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
			if (Pixels[i] == find) Pixels[i] = replace;
		}
		return *this;
	}

	ImageBlock Graphics::ReadPixelsRect(int x, int y, int r, int b)
	{
		ImageBlock ret;
		int width, height;
		if (!PreFitAreaGetWH(x, y, r, b, width, height)) return ret;
		ret.w = width;
		ret.h = height;

		for(int i = 0; i < height; i++)
		{
			auto row = ReadPixelsRow(x, i, width);
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
		fs(std::fstream(std::string("/dev/") + fbdev, std::ios::in | std::ios::out)),
		Verbose(Verbose)
	{
		if (Verbose)
		{
			std::cout << "[INFO] Opening `/dev/" << fbdev << "` in input/output mode.\n";
		}
		try
		{
			fs.exceptions(std::ios::badbit | std::ios::failbit);
		} catch (const std::ios::failure& e)
		{
			std::cerr << "[WARN] Could not open `/dev/" << fbdev << "` for input/output mode, opening in output mode: `" << e.what() << "`\n";
			fs = std::fstream(std::string("/dev/") + fbdev, std::ios::out);
			if (Verbose)
			{
				std::cout << "[INFO] Opening `/dev/" << fbdev << "` in output mode.\n";
			}
			fs.exceptions(std::ios::badbit | std::ios::failbit);
			if (Verbose)
			{
				std::cout << "[INFO] Opened `/dev/" << fbdev << "` in output mode.\n";
			}
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

	void Graphics::SetDrawPos(int x, int y)
	{
		if (!BackBufferMode && (BBReadPosX != x || BBReadPosY !=y))
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
		if (!BackBufferMode && (BBReadPosX != x || BBReadPosY !=y))
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
		if (Verbose)
		{
			std::cout << "[INFO] Writting " << Count << " pixels from pointer " << std::hex << pixels << ".\n";
		}
		if (BackBufferMode)
		{
			fs.write(reinterpret_cast<const char*>(pixels), Count * 4);
		}
		else
		{
			memcpy(&BackBuffer.get()[BBWritePosY * BackBuffer->w + BBWritePosX], pixels, Count * 4);
		}
	}
	
	void Graphics::WriteData(const std::vector<uint32_t>& pixels)
	{
		WriteData(&pixels[0], pixels.size());
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
		ret.resize(count);
		SetReadPos(x, y);
		if (BackBufferMode)
		{
			memcpy(&ret[0], &BackBuffer.get()[BBReadPosY * BackBuffer->w + BBReadPosX], count * 4);
		}
		else
		{
			fs.read(reinterpret_cast<char*>(&ret[0]), count * 4);
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
			FillRect(0, 0, Width - 1, Height - 1, 0xFFFFFFFF);
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
	
	void Graphics::DrawHLineXor(int x, int y1, int y2)
	{
		FillRectXor(x, y1, x, y2);
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
			SetDrawPos(iy, x);
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
		if (ops == 0) DrawImage(ib, x, y, w, h, srcx, srcy); return;
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

	void Graphics::DrawGlyph(int x, int y, uint32_t Glyph, bool Transparent, uint32_t GlyphColor)
	{
		try
		{
			auto& GlyphImage = Glyphs.at(Glyph);
			if (!Transparent && GlyphColor == 0)
			{
				DrawImage(GlyphImage, x, y);
			}
			else
			{
				if (Transparent) DrawImageAnd(GlyphImage, x, y);
				if (GlyphColor != 0)
				{
					ImageBlock NewGlyphImage = GlyphImage;
					NewGlyphImage.InvertPixelColors().ReplacePixelColors(0xFFFFFFFF, GlyphColor);
					DrawImageOr(NewGlyphImage, x, y);
				}
			}

			// 因为用过这个字，所以将其缓存优先级设为最高
			auto& UsageIndex = GlyphToUsageIndices.at(Glyph);
			std::swap(GlyphsUsage[0], GlyphsUsage[UsageIndex]);
			UsageIndex = 0;
		}
		catch (const std::out_of_range&)
		{
			// 生成字体
			auto& GlyphImage = Glyphs[Glyph];
			ExtractGlyph(GlyphImage, Glyph, 0x00000000, 0xFFFFFFFF);

			// 添加到缓存统计
			GlyphToUsageIndices[Glyph] = 0;
			GlyphsUsage.insert(GlyphsUsage.begin(), Glyph);

			// 统计内存使用量（只统计字体的）
			GlyphMapMemoryUsage += GlyphImage.GetSizeInBytes();
			while (GlyphMapMemoryUsage > GlyphMapMaxMemoryUsage)
			{
				// 移除 GlyphsUsage 最末尾的字体。
				auto LastUsedGlyph = GlyphsUsage.back();
				GlyphsUsage.pop_back();
				GlyphToUsageIndices.erase(LastUsedGlyph);
				GlyphMapMemoryUsage -= Glyphs.at(LastUsedGlyph).GetSizeInBytes();
				Glyphs.erase(LastUsedGlyph);
			}
		}
	}
}