
#include "graphics.hpp"
#include "gui.hpp"
#include "gpio.hpp"

#if !defined(_MSC_VER)
#include <sys/mount.h>
#endif

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <filesystem>

#include <chrono>
#include <thread>

#if defined(_MSC_VER)
#include "tvos.hpp"
#include <Windows.h>
#endif

using namespace TVOS;

int main(int argc, char** argv, char** envp)
{
	const int ResoW = 480;
	const int ResoH = 272;

	bool Mounted = false;

	WriteGPIOE(0, true);

#if !defined(_MSC_VER)
	auto FB = Graphics(ResoW, ResoH, false);
#else
	auto FB = MyTestApp(true);
#endif
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

	while (true)
	{
#if !defined(_MSC_VER)
		if (!std::filesystem::exists(std::filesystem::path("/dev/mmcblk0p1")))
#else
		if (GetAsyncKeyState(VK_SPACE))
#endif
		{
			if (Mounted)
			{
#if !defined(_MSC_VER)
				if (umount2("/mnt/sdcard", MNT_FORCE) == 0)
#else
				if (1)
#endif
				{
					Mounted = false;

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
					Sub->Alignment = AlignmentType::CenterTop;
					Sub->SetCaption("A5-MiniTV 小电视");

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
					Prompt->Alignment = AlignmentType::CenterCenter;
					Prompt->SetCaption("请插入 SD 卡");

				}
				GUI.ArrangeElements(0, 0, FB.GetWidth(), FB.GetHeight());
				FB.ClearScreen(0);
			}
		}
		else
		{
#if !defined(_MSC_VER)
			auto SDCardPath = std::filesystem::path("/mnt/sdcard");
			if (!std::filesystem::exists(SDCardPath))
			{
				std::filesystem::create_directories(SDCardPath);
			}
#endif

			if (!Mounted)
			{
#if !defined(_MSC_VER)
				if (mount("/dev/mmcblk0p1", "/mnt/sdcard", "vfat", MS_REMOUNT, "defaults,nofail") == 0)
#else
				if (1)
#endif
				{
					Mounted = true;
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
					Sub->Alignment = AlignmentType::CenterTop;
					Sub->SetCaption("请选择要播放的曲目");

					auto ListView = std::make_shared<UIElementListView>(FB, "ListView");
					GUI.InsertElement(ListView);
					ListView->XMargin = 0;
					ListView->YMargin = 0;
					ListView->XBorder = 0;
					ListView->YBorder = 0;
					ListView->XPadding = 0;
					ListView->YPadding = 0;
					ListView->BorderColor = 0xFFFFFFFF;
					ListView->ExpandToParentX = true;
					ListView->ExpandToParentY = true;
					ListView->LineBreak = false;
					ListView->Transparent = true;
					ListView->Alignment = AlignmentType::LeftTop;


				}
				GUI.ArrangeElements(0, 0, FB.GetWidth(), FB.GetHeight());
				FB.ClearScreen(0);
			}


		}

		GUI.Render();
#if !defined(_MSC_VER)
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
#else
		FB.RefreshFB();
		FB.ProcessMessageNonBlocking();
		if (FB.GetWindowIsDestroyed()) break;
#endif
	}

	return 0;
}


