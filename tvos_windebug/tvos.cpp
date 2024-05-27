
#include <iostream>
#include <filesystem>

#include "tvos.hpp"

using namespace TVOS;

int main()
{
	auto FB = MyTestApp(false);
	auto GUI = UIElementBase(FB, "root");
	FB.ClearScreen(0);

	GUI.XMargin = 10;
	GUI.YMargin = 10;
	GUI.XBorder = 2;
	GUI.YBorder = 2;
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
		// Sub->LineBreak = true;
		Sub->Transparent = true;
		Sub->SetCaption(Sub->GetName());

	}

	GUI.ArrangeElements(0, 0, FB.GetWidth(), FB.GetHeight());
	GUI.Render(0, 0, FB.GetWidth(), FB.GetHeight());

	FB.RefreshFB();
	FB.MainLoop();
}