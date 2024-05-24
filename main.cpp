
#include "graphics.hpp"

#include <cstdio>
#include <cstdlib>
#include <iostream>

using namespace TVOS;

int main(int argc, char** argv, char** envp)
{
	auto FB = Graphics(false);

	for(int i = 0; i < 10000; i++)
	{
		int putx = std::rand() % FB.GetWidth();
		int puty = std::rand() % FB.GetHeight();
		FB.PutPixel(putx, puty, MakeColor(std::rand() % 256, std::rand() % 256, std::rand() % 256));
	}

	return 0;
}


