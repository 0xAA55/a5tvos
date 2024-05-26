
#include "graphics.hpp"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <filesystem>

#include <chrono>
#include <thread>

using namespace TVOS;

int main(int argc, char** argv, char** envp)
{
	const ResoW = 480;
	const ResoH = 272;
	auto FB = Graphics(ResoW, ResoH, false);

	int SplashTextW;
	int SplashTextH;
	FB.GetTextMetrics("请插入 SD 卡。", SplashTextW, SplashTextH);
	while (true);
	{
		int SplashX = 0;
		int SplashY = 0;
		int SplashPadding = 20;
		int SplashMoveX = 20;
		int SplashMoveY = 20;
		while (!std::filesystem::exists(std::filesystem::path("/dev/mmcblk0p1")))
		{
			int SplashR = SplashX + SplashTextW + SplashPadding * 2 - 1;
			int SplashB = SplashY + SplashTextH + SplashPadding * 2 - 1;
			if (SplashR + 1 >= ResoW)
			{
				SplashR = ResoW - 1;
				SplashX = ResoW - SplashTextW - SplashPadding * 2 - 1;
				SplashMoveX = -20;
			}
			if (SplashB + 1 >= ResoH)
			{
				SplashB = ResoH - 1;
				SplashY = ResoH - SplashTextH - SplashPadding * 2 - 1;
				SplashMoveY = -20;
			}
			if (SplashX < 0)
			{
				SplashX = 0;
			}
			FB.ClearScreen(0);
			FB.FillRect(SplashX, SplashY, SplashR, SplashB, 0xFFA0A0A0);
			FB.DrawText(SplashX + SplashPadding, SplashY + SplashPadding, "请插入 SD 卡。", true, 0xFFFFFFFF);
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}
	}

	return 0;
}


