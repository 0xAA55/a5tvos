#pragma once

#include <string>
#include <memory>

namespace DIBWin
{
	class HWNDInternal;
	class HDCInternal;

	struct BMIF32
	{
		uint32_t biSize = 40;
		int32_t biWidth;
		int32_t biHeight;
		uint16_t biPlanes = 1;
		uint16_t biBitCount = 32;
		uint32_t biCompression = 3;
		uint32_t biSizeImage;
		int32_t biXPelsPerMeter = 3000;
		int32_t biYPelsPerMeter = 3000;
		uint32_t biClrUsed = 0;
		uint32_t biClrImportant = 0;
		uint32_t bfRed = 0x00FF0000;
		uint32_t bfGreen = 0x0000FF00;
		uint32_t bfBlue = 0x000000FF;
	};

	class Window
	{
	protected:
		std::shared_ptr<HWNDInternal> Ptr_hWnd;
		std::shared_ptr<HDCInternal> Ptr_hDC;
		int CreationWidth = 800;
		int CreationHeight = 480;
		void* FBPtr = nullptr;
		static const std::string ClassName;
		bool WindowIsDestroyed = false;

		static BMIF32 CreateBMIF(int Width, int Height);
		const HWNDInternal& GetWindow() const;
		const HDCInternal& GetDC() const;

		Window();
	public:
		Window(const Window& w) = default;
		Window(Window&& w) = default;
		Window(int Width, int Height, const std::string& Title);

		void* GetFBPtr();
		void RefreshFB();

		void ProcessMessage();
		virtual int MainLoop();

	public:
		static size_t OnWndProc(Window* window, void *hWnd, uint32_t Msg, size_t WParam, size_t LParam);
	};
}
