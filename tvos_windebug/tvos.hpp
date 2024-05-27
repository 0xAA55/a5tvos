#pragma once

#include <graphics.hpp>
#include <gui.hpp>
#include "dibwin.hpp"

class MyTestApp :
	public DIBWin::Window,
	public TVOS::Graphics
{
public:
	MyTestApp();
	MyTestApp(bool Verbose);
	MyTestApp(int Width, int Height);
	MyTestApp(int Width, int Height, bool Verbose);

	void RefreshFB();
};

MyTestApp::MyTestApp() :
	MyTestApp(480, 272, Verbose)
{
}

MyTestApp::MyTestApp(bool Verbose) :
	MyTestApp(480, 272, Verbose)
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
