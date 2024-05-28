
#include "graphics.hpp"
#include "gui.hpp"
#include "gpio.hpp"

#include <sys/mount.h>

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

	bool Mounted = false;

	WriteGPIOE(0, true);

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

	while (true);
	{
		if (!std::filesystem::exists(std::filesystem::path("/dev/mmcblk0p1")))
		{
			if (Mounted)
			{
				GUI.ClearElements();

				auto Sub = std::make_shared<UIElementLabel>(FB, "Title");
				GUI.InsertElement(Sub);
				Sub->XMargin = 0;
				Sub->YMargin = 0;
				Sub->XBorder = 0;
				Sub->YBorder = 1;
				Sub->XPadding = 2;
				Sub->YPadding = 2;
				Sub->BorderColor = 0xFFFFFFFF;
				Sub->ExpandToParentX = true;
				Sub->LineBreak = false;
				Sub->Transparent = true;
				Sub->Alignment = CenterTop;
				Sub->SetCaption("A5-MiniTV 小电视");
				if (umount("/mnt/sdcard") == 0)
				{
					Mounted = false;

					auto Prompt = std::make_shared<UIElementLabel>(FB, "Prompt");
					GUI.InsertElement(Prompt);
					Prompt->XMargin = 0;
					Prompt->YMargin = 0;
					Prompt->XBorder = 0;
					Prompt->YBorder = 0;
					Prompt->XPadding = 0;
					Prompt->YPadding = 0;
					Prompt->BorderColor = 0xFFFFFFFF;
					Prompt->ExpandToParentX = true;
					Prompt->ExpandToParentY = true;
					Prompt->LineBreak = false;
					Prompt->Transparent = true;
					Prompt->Alignment = CenterCenter;
					Prompt->SetCaption("请插入 SD 卡");
				}
				else
				{

					auto Prompt = std::make_shared<UIElementLabel>(FB, "Prompt");
					GUI.InsertElement(Prompt);
					Prompt->XMargin = 0;
					Prompt->YMargin = 0;
					Prompt->XBorder = 0;
					Prompt->YBorder = 0;
					Prompt->XPadding = 0;
					Prompt->YPadding = 0;
					Prompt->BorderColor = 0xFFFFFFFF;
					Prompt->ExpandToParentX = true;
					Prompt->ExpandToParentY = true;
					Prompt->LineBreak = false;
					Prompt->Transparent = true;
					Prompt->SetCaption("挂载 SD 卡失败。");
				}
				GUI.ArrangeElements(0, 0, FB.GetWidth(), FB.GetHeight());
			}
		}
		else
		{
			auto SDCardPath = std::filesystem::path("/mnt/sdcard");
			if (!std::filesystem::exists(SDCardPath))
			{
				std::filesystem::create_directories(SDCardPath);
			}

			if (!Mounted)
			{
				if (mount("/dev/mmcblk0p1", "/mnt/sdcard", "vfat", MS_REMOUNT, "defaults,nofail") == 0)
				{
					Mounted = true;
				}
			}

			
		}

		GUI.Render();
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}

	return 0;
}


