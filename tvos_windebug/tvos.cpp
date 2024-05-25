
#include <graphics.hpp>
#include "dibwin.hpp"

#include <iostream>

using namespace TVOS;
using namespace DIBWin;

class MyTestApp :
	public Window,
	public Graphics
{
public:
	MyTestApp();
	MyTestApp(int Width, int Height);
	MyTestApp(int Width, int Height, bool Verbose);

	void RefreshFB();
};

int main()
{
	auto FB = MyTestApp();
	
	FB.FillRectXor(200, 200, 800 - 200, 480 - 200);
	FB.DrawText(200, 200, "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ", false, 0);

	FB.RefreshFB();
	FB.MainLoop();
}

MyTestApp::MyTestApp() :
	MyTestApp(800, 480)
{
}

MyTestApp::MyTestApp(int Width, int Height) :
	Window(Width, Height, "TVOS"),
	Graphics(nullptr, Width, Height, Verbose)
{
}

MyTestApp::MyTestApp(int Width, int Height, bool Verbose) :
	Window(Width, Height, "TVOS"),
	Graphics(nullptr, Width, Height, Verbose)
{
}

void MyTestApp::RefreshFB()
{
	memcpy(GetFBPtr(), &BackBuffer->Pixels[0], BackBuffer->GetStride() * BackBuffer->h);
	Window::RefreshFB();
}
