
#include <iostream>

#include "tvos.hpp"

using namespace TVOS;

int main()
{
	auto FB = MyTestApp(true);
	
	FB.ClearScreen(0xFFFFFFFF);
	FB.DrawText(200, 200, "啊!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ", true, 0xFFFF0000);

	FB.RefreshFB();
	FB.MainLoop();
}