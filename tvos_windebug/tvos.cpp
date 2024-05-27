
#include <iostream>
#include <filesystem>

#include "tvos.hpp"

using namespace TVOS;

int main()
{
	auto FB = MyTestApp(true);
	auto GUI = UIElementBase(FB, "root");
	FB.ClearScreen(0);

	GUI.XMargin = 10;
	GUI.YMargin = 10;
	GUI.XBorder = 2;
	GUI.YBorder = 2;
	GUI.BorderColor = 0xFFFFFFFF;
	GUI.FillColor = 0;
	




	GUI.Render(0, 0, FB.GetWidth(), FB.GetHeight());

	FB.RefreshFB();
	FB.MainLoop();
}