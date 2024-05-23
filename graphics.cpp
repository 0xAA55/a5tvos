#include "graphics.hpp"

#include <cstring>

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

	int ImageBlock::GetStride() const
	{
		return w * 4;
	}

	ImageBlock Graphics::ReadPixels(int x, int y, int r, int b)
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

	Graphics::Graphics(const std::string& fbdev) :
		ifs(std::ifstream(std::string("/dev/") + fbdev, std::ios::binary)),
		ofs(std::ofstream(std::string("/dev/") + fbdev, std::ios::binary))
	{
		ifs.exceptions(std::ios::badbit | std::ios::failbit);
		ofs.exceptions(std::ios::badbit | std::ios::failbit);

		GetFBSize(fbdev, Width, Height);
		Stride = GetFBStride(fbdev);
	}
	
	Graphics::Graphics(const std::string& fbdev, int width, int height) :
		Graphics(fbdev)
	{
		Width = width;
		Height = height;
		Stride = GetFBStride(fbdev);
	}

	Graphics::Graphics() : Graphics("fb0")
	{
	}

	Graphics::Graphics(int width, int height) : Graphics("fb0", width, height)
	{
	}

	std::string Graphics::ReadFile(const std::string& f)
	{
		std::ifstream r(f, std::ios::binary);
		r.exceptions(std::ios::badbit | std::ios::failbit);
		r.seekg(0, std::ios::end);
		int size = r.tellg();
		std::string buffer(size, '\0');
		r.seekg(0);
		r.read(&buffer[0], size); 
		return buffer;
	}

	void Graphics::GetFBSize(const std::string& fbdev, int& Width, int& Height)
	{
		Width = Height = 0;
		auto StringSize = ReadFile(std::string("/sys/class/graphics/") + fbdev + "/virtual_size");
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
		return std::stoi(ReadFile(std::string("/sys/class/graphics/") + fbdev + "/stride"));
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
		int Row = int(y) * Stride;
		ofs.seekp(Row + x * 4);
	}

	void Graphics::SetReadPos(int x, int y)
	{
		int Row = int(y) * Stride;
		ifs.seekg(Row + x * 4);
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
		ofs.write(reinterpret_cast<const char*>(pixels), Count * (sizeof pixels[0]));
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
		ifs.read(reinterpret_cast<char*>(&ret[0]), count * 4);
		return ret;
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

	void Graphics::DrawImages(const ImageBlock& ib, int x, int y, int w, int h, int srcx, int srcy)
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
}