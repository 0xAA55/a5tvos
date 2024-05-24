
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
		int putr = std::rand() % FB.GetWidth();
		int putb = std::rand() % FB.GetHeight();
		std::cout << "[INFO] Drawing rectangle at " << putx << ", " << puty << ", " << putr << ", " << putb << ".\n";
		FB.DrawRectXor(putx, puty, putr, putb);
	}

	return 0;
}


