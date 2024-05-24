
#include <cstdio>
#include <cstdlib>

#include "graphics.hpp"

using namespace TVOS;

int main(int argc, char** argv, char** envp)
{
	auto FB = Graphics(true);

	for(int i = 0; i < 10000; i++)
	{
		FB.PutPixel(std::rand() % FB.GetWidth(), std::rand() % FB.GetHeight(),
			MakeColor(std::rand() % 256, std::rand() % 256, std::rand() % 256));
	}

	return 0;
}


