
#include <iostream>
#include <filesystem>

#include "tvos.hpp"

using namespace TVOS;

int main()
{
	auto FB = MyTestApp(true);
	








	FB.ClearScreen(0);


	FB.DrawText(200, 200, "请插入 SD 卡", true, 0xFFFFFFFF);

	FB.RefreshFB();
	FB.MainLoop();
}