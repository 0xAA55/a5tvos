
#include "graphics.hpp"

#include <cstdio>
#include <cstdlib>
#include <iostream>

using namespace TVOS;

int main(int argc, char** argv, char** envp)
{
	// auto FB = Graphics(480, 272, false);
	auto FB = Graphics(false);

	for(int i = 0; i < 50; i++)
	{
		int putx = std::rand() % FB.GetWidth();
		int puty = std::rand() % FB.GetHeight();
		// FB.DrawText(putx, puty, "测试文本绘制", true, MakeColor(std::rand() % 256, std::rand() % 256, std::rand() % 256));
		FB.DrawText(putx, puty, "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ", false, 0);
	}
	FB.ClearScreen(0);
	FB.DrawText(200, 100, "测试文本绘制", false, 0);

	return 0;
}


