#include "dibwin.hpp"

#include <Windows.h>
#include <memory>
#include <stdexcept>

namespace DIBWin
{
	template <typename H>
	class HandleInternal
	{
	protected:
		H Handle;
	public:
		HandleInternal(H Handle) : Handle(Handle)
		{
			if (Handle == NULL) throw std::invalid_argument("Handle is NULL.");
		}

		operator H() const { return Handle; }
	};

	class HWNDInternal : public HandleInternal<HWND>
	{
	public:
		using HandleInternal<HWND>::HandleInternal;

		~HWNDInternal() { DestroyWindow(Handle); }
	};
	class HDCInternal : public HandleInternal<HDC>
	{
	public:
		using HandleInternal<HDC>::HandleInternal;

		~HDCInternal() { DeleteDC(Handle); }
	};

	BMIF32 Window::CreateBMIF(int Width, int Height)
	{
		auto ret = BMIF32();
		ret.biWidth = Width;
		ret.biHeight = -Height;
		ret.biSizeImage = Width * Height * 4;
		return ret;
	}

	const HWNDInternal& Window::GetWindow() const
	{
		return *Ptr_hWnd;
	}

	const HDCInternal& Window::GetDC() const
	{
		return *Ptr_hDC;
	}

	static LRESULT CALLBACK WndProcA(HWND hWnd, uint32_t Msg, size_t WParam, size_t LParam)
	{
		auto* window = reinterpret_cast<Window*>(GetWindowLongPtrA(hWnd, 0));
		return Window::OnWndProc(window, hWnd, Msg, WParam, LParam);
	}

	const std::string Window::ClassName = "DIBWin_Window";

	Window::Window()
	{
		WNDCLASSEXA WCEx =
		{
			sizeof(WNDCLASSEXA),
			0,
			WNDPROC(&WndProcA),
			0,
			sizeof (Window*),
			GetModuleHandleA(NULL),
			NULL,
			NULL,
			HBRUSH((COLOR_WINDOW) + 1),
			NULL,
			ClassName.c_str(),
			NULL
		};
		RegisterClassExA(&WCEx);
	}

	size_t Window::OnWndProc(Window* window, void* hWnd, uint32_t Msg, size_t WParam, size_t LParam)
	{
		switch (Msg)
		{
		case WM_CREATE:
			do
			{
				auto* CreationParam = reinterpret_cast<CREATESTRUCTA*>(LParam);
				SetWindowLongPtrA(HWND(hWnd), 0, size_t(CreationParam->lpCreateParams));
			} while (false);
			break;
		case WM_PAINT:
			do
			{
				PAINTSTRUCT ps;
				auto hDC = BeginPaint(HWND(hWnd), &ps);
				BitBlt(hDC, 0, 0, window->CreationWidth, window->CreationHeight, window->GetDC(), 0, 0, SRCCOPY);
				EndPaint(HWND(hWnd), &ps);
			} while (false);
			break;
		case WM_DESTROY:
			SetWindowLongPtrA(HWND(hWnd), 0, 0);
			window->WindowIsDestroyed = true;
			break;
		default:
			return DefWindowProcA(HWND(hWnd), Msg, WParam, LParam);
		}
		return 0;
	}

	Window::Window(int Width, int Height, const std::string& Title) :
		Window()
	{
		Ptr_hWnd = std::make_shared<HWNDInternal>(CreateWindowExA(
			0,
			ClassName.c_str(), Title.c_str(),
			WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
			CW_USEDEFAULT, CW_USEDEFAULT,
			Width, Height,
			NULL,
			NULL,
			GetModuleHandleA(NULL),
			this));
		CreationWidth = Width;
		CreationHeight = Height;
		auto BMIF = CreateBMIF(Width, Height);
		Ptr_hDC = std::make_shared<HDCInternal>(CreateCompatibleDC(NULL));
		auto hBmp = CreateDIBSection(
			NULL,
			reinterpret_cast<BITMAPINFO*>(&BMIF),
			DIB_RGB_COLORS,
			&FBPtr,
			NULL,
			0);
		if (!hBmp) throw std::runtime_error("`CreateDIBSection()` failed.");
		DeleteObject(SelectObject(GetDC(), hBmp));
		DeleteObject(hBmp);

		RECT rcOuter, rcInner;
		GetWindowRect(GetWindow(), &rcOuter);
		GetClientRect(GetWindow(), &rcInner);
		auto BorderWidth = (rcOuter.right - rcOuter.left) - (rcInner.right - rcInner.left);
		auto BorderHeight = (rcOuter.bottom - rcOuter.top) - (rcInner.bottom - rcInner.top);
		MoveWindow(GetWindow(), rcOuter.left, rcOuter.top, CreationWidth + BorderWidth, CreationHeight + BorderHeight, FALSE);
		ShowWindow(GetWindow(), SW_SHOW);
	}

	void* Window::GetFBPtr()
	{
		return FBPtr;
	}

	void Window::RefreshFB()
	{
		auto hWndDC = ::GetDC(GetWindow());
		BitBlt(hWndDC, 0, 0, CreationWidth, CreationHeight, GetDC(), 0, 0, SRCCOPY);
		ReleaseDC(GetWindow(), hWndDC);
	}

	void Window::ProcessMessage()
	{
		MSG msg;
		if (GetMessageA(&msg, GetWindow(), 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessageA(&msg);
		}
	}

	void Window::ProcessMessageNonBlocking()
	{
		MSG msg;
		while (PeekMessageA(&msg, GetWindow(), 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageA(&msg);
		}
	}

	bool Window::GetWindowIsDestroyed() const
	{
		return WindowIsDestroyed;
	}

	int Window::MainLoop()
	{
		while (!WindowIsDestroyed)
		{
			ProcessMessage();
		}
		return 0;
	}
}

