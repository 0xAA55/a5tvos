
#include <iostream>

#include "tvos.hpp"

using namespace TVOS;

int main()
{
	auto FB = MyTestApp(true);
	
	FB.ClearScreen(0xFFFFFFFF);
	FB.DrawText(200, 200, "测试文本绘制", true, 0xFF00FF00);

	FB.RefreshFB();
	FB.MainLoop();
}