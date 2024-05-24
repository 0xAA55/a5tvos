
#include "graphics.hpp"

#include <cstdio>
#include <cstdlib>
#include <iostream>

using namespace TVOS;

int main(int argc, char** argv, char** envp)
{
	auto FB = Graphics(true);

	int putx = 0;
	int puty = 0;
	for(int i = 0; i < 10000; i++)
	{
		std::cout << "[INFO] Putting pixel at " << putx << "x" << puty << "\n";
		FB.PutPixel(putx, puty, MakeColor(255, 255, 255));
		putx = std::rand() % FB.GetWidth();
		puty = std::rand() % FB.GetHeight();
	}

	return 0;
}


