
#include "graphics.hpp"

#include <cstdio>
#include <cstdlib>
#include <iostream>

using namespace TVOS;

int main(int argc, char** argv, char** envp)
{
	auto FB = Graphics(480, 272, false);

	for(int i = 0; i < 1000; i++)
	{
		int putx = std::rand() % FB.GetWidth();
		int puty = std::rand() % FB.GetHeight();
		int putr = std::rand() % FB.GetWidth();
		int putb = std::rand() % FB.GetHeight();
		FB.FillRectXor(putx, puty, putr, putb);
	}


	return 0;
}


