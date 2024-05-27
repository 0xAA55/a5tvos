
#include "graphics.hpp"
#include "gui.hpp"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <filesystem>

#include <chrono>
#include <thread>

using namespace TVOS;

int main(int argc, char** argv, char** envp)
{
	const int ResoW = 480;
	const int ResoH = 272;
	auto FB = Graphics(ResoW, ResoH, false);
	auto GUI = UIElementBase(FB, "root");
	FB.ClearScreen(0);

	GUI.XMargin = 2;
	GUI.YMargin = 2;
	GUI.XBorder = 2;
	GUI.YBorder = 2;
	GUI.XPadding = 0;
	GUI.YPadding = 0;
	GUI.BorderColor = 0xFFFFFFFF;
	GUI.FillColor = 0;
	GUI.Transparent = true;
	GUI.ExpandToParentX = true;
	GUI.ExpandToParentY = true;

	for (int i = 0; i < 100; i++)
	{
		auto Sub = std::make_shared<UIElementLabel>(FB, std::string("test") + std::to_string(i));
		GUI.InsertElement(Sub);
		Sub->XMargin = 2;
		Sub->YMargin = 2;
		Sub->XBorder = 1;
		Sub->YBorder = 1;
		Sub->XPadding = 2;
		Sub->YPadding = 2;
		Sub->BorderColor = 0xFFFFFFFF;
		Sub->ExpandToParentX = true;
		Sub->LineBreak = false;
		Sub->Transparent = true;
		Sub->SetCaption(Sub->GetName());
	}

	GUI.ArrangeElements(0, 0, FB.GetWidth(), FB.GetHeight());
	GUI.Render();

	while (true);
	{
		
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}

	return 0;
}


