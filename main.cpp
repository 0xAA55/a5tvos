
#include "graphics.hpp"
#include "gui.hpp"
#include "gpio.hpp"

#if !defined(_MSC_VER)
#include <sys/mount.h>
#include <unistd.h>
#endif

#include <cstdio>
#include <cstdlib>
#include <csignal>
#include <iostream>
#include <filesystem>

#include <chrono>
#include <thread>

#if defined(_MSC_VER)
#include "tvos.hpp"
#include <Windows.h>
std::string GetLastErrorAsString()
{
	DWORD errorMessageID = GetLastError();
	if (errorMessageID == 0) return "";

	LPSTR messageBuffer = nullptr;

	size_t size = FormatMessageA
	(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorMessageID,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&messageBuffer,
		0,
		NULL
	);

	auto ret = std::string(messageBuffer, size);
	LocalFree(messageBuffer);
	return ret;
}
#endif

#if !defined(_MSC_VER)
pid_t popen2(const char* command, int* infp, int* outfp)
{
	const int READ = 0;
	const int WRITE = 1;
	int p_stdin[2], p_stdout[2];
	pid_t pid;

	if (pipe(p_stdin) != 0 || pipe(p_stdout) != 0)
		return -1;

	pid = fork();

	if (pid < 0)
		return pid;
	else if (pid == 0)
	{
		close(p_stdin[WRITE]);
		dup2(p_stdin[READ], READ);
		close(p_stdout[READ]);
		dup2(p_stdout[WRITE], WRITE);

		execl("/bin/sh", "sh", "-c", command, NULL);
		perror("execl");
		exit(1);
	}

	if (infp == NULL)
		close(p_stdin[WRITE]);
	else
		*infp = p_stdin[WRITE];

	if (outfp == NULL)
		close(p_stdout[READ]);
	else
		*outfp = p_stdout[READ];

	return pid;
}
#else
using pid_t = int;
pid_t popen2(const char* command, int* infp, int* outfp)
{
	PROCESS_INFORMATION ProcInfo;
	STARTUPINFOA StartupInfo =
	{
		sizeof(STARTUPINFOA), NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0,
		STARTF_USESTDHANDLES,
		0,
		0,
		NULL,
		GetStdHandle(STD_INPUT_HANDLE),
		GetStdHandle(STD_OUTPUT_HANDLE),
		GetStdHandle(STD_ERROR_HANDLE)
	};
	if (CreateProcessA(NULL, const_cast<char*>(command), NULL, NULL, TRUE, 0, 0, NULL, &StartupInfo, &ProcInfo))
	{
		CloseHandle(ProcInfo.hThread);
		CloseHandle(ProcInfo.hProcess);
		return pid_t(ProcInfo.dwProcessId);
	}
	else
	{
		std::cerr << GetLastErrorAsString();
		exit(-1);
	}
}
#endif

using namespace TVOS;

size_t GetFileSize(const std::string& File)
{
	FILE* fp = fopen(File.c_str(), "rb");
	if (fp)
	{
		fseek(fp, 0, SEEK_END);
		auto size = ftell(fp);
		fclose(fp);
		return size;
	}
	return 0;
}

pid_t PlayVideo(const std::string& VideoFile)
{
	char buf[4096];

#ifndef _MSC_VER
	// system("tinymix set 1 0"); // 调试时设置音量为 0
	system("tinymix set 2 1");
	system("tinymix set 13 0");

	auto FileSize = GetFileSize(VideoFile);
	if (FileSize > 0 && FileSize <= 16384 * 1024)
	{
		snprintf(buf, sizeof buf, "cat %s > /dev/null", VideoFile.c_str());
		system(buf);
	}
#endif

#ifndef _MSC_VER
	snprintf(buf, sizeof buf, "ffmpeg -hide_banner -loglevel panic -i %s -an -pix_fmt bgra -f fbdev /dev/fb0 -vn -f wav pipe:1 -ar 44100 -ac 1 | tinyplay stdin -r 44100 -c 1", VideoFile.c_str());
	printf("%s\n", buf);
	return popen2(buf, nullptr, nullptr);
#else
	snprintf(buf, sizeof buf, "ffplay %s", VideoFile.c_str());
	printf("%s\n", buf);
	return popen2(buf, nullptr, nullptr);
#endif
}

void StopPlay(pid_t& pid)
{
#ifdef _MSC_VER
	const auto player = OpenProcess(PROCESS_TERMINATE, false, DWORD(pid));
	TerminateProcess(player, 1);
	CloseHandle(player);
#else
	kill(pid, SIGINT);
#endif
	pid = -1;
}

bool IsPlaying(pid_t pid)
{
#ifdef _MSC_VER
	const auto player = OpenProcess(PROCESS_QUERY_INFORMATION, false, DWORD(pid));
	DWORD ExitCode = 0;
	GetExitCodeProcess(player, &ExitCode);
	CloseHandle(player);
	return ExitCode == STILL_ACTIVE;
#else
	return kill(pid, 0) == 0;
#endif
}

int main(int argc, char** argv, char** envp)
{
	const int ResoW = 480;
	const int ResoH = 272;

	bool Mounted = false;

	WriteGPIOE(0, true);

	bool Key1 = false;
	bool Key2 = false;
	bool Key3 = false;
	bool Key4 = false;

	bool NeedRedraw = true;

	pid_t PlayerProcess = -1;

#if !defined(_MSC_VER)
	auto FB = Graphics(ResoW, ResoH, false);
	// FB.SetBackBufferMode();
#else
	auto FB = MyTestApp(false);
#endif
	auto GUI = UIElementBase(FB, "root");

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
				if (PlayerProcess != -1) StopPlay(PlayerProcess);

#if !defined(_MSC_VER)
				umount2("/mnt/sdcard", MNT_FORCE);
#endif
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
				Sub->Transparent = false;
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

				NeedRedraw = true;
			}
		}
		else
		{
#if !defined(_MSC_VER)
			std::string media_path = "/mnt/sdcard";
#else
			std::string media_path = "testsdcard";
#endif

			auto SDCardPath = std::filesystem::path(media_path);
			if (!std::filesystem::exists(SDCardPath))
			{
				std::filesystem::create_directories(SDCardPath);
			}

			if (!Mounted)
			{
#if !defined(_MSC_VER)
				int m = mount("/dev/mmcblk0p1", media_path.c_str(), "vfat", 0, "");
				if (m != 0)
				{
					perror("mount()");
					if (errno == EBUSY) m = mount("/dev/mmcblk0p1", media_path.c_str(), "vfat", MS_REMOUNT, "");
					if (m != 0)
					{
						perror("mount()");
					}
				}
				if (m == 0)
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
					Sub->Transparent = false;
					Sub->Alignment = AlignmentType::CenterTop;
					Sub->SetCaption("请选择要播放的曲目");

					auto ListView = std::make_shared<UIElementListView>(FB, "ListView");
					GUI.InsertElement(ListView);
					ListView->XMargin = 10;
					ListView->YMargin = 10;
					ListView->XBorder = 1;
					ListView->YBorder = 1;
					ListView->XPadding = 1;
					ListView->YPadding = 1;
					ListView->BorderColor = 0xFFC0C0C0;
					ListView->ExpandToParentX = true;
					ListView->ExpandToParentY = true;
					ListView->LineBreak = false;
					ListView->Transparent = true;
					ListView->Alignment = AlignmentType::LeftTop;
					for (auto& directory : std::filesystem::directory_iterator(media_path))
					{
						// 跳过文件夹
						if (directory.is_directory()) continue;

						auto p = directory.path();
						auto FileNameString = p.filename().string();
						if (p.extension() == ".mp4" || p.extension() == ".MP4")
						{
							auto ListItem = std::make_shared<UIElementListItem>(FB, FileNameString);
							ListView->AddItem(FileNameString, FileNameString);
						}
					}

					NeedRedraw = true;
				}
				else
				{
#if !defined(_MSC_VER)
					perror("mount()");
#endif
				}
			}

			if (GUI.count("ListView"))
			{
				auto& ListView = dynamic_cast<UIElementListView&>(*GUI.at("ListView"));
				if (ReadGPIOE(1))
				{
					if (Key1 == false)
					{
						Key1 = true;
						auto VideoFile = (SDCardPath / ListView.GetSelectedItem().GetCaption()).string();
						FB.ClearScreen(0);
						if (PlayerProcess != -1) StopPlay(PlayerProcess);
						PlayerProcess = PlayVideo(VideoFile);
					}
				}
				else
				{
					Key1 = false;
				}
				if (ReadGPIOE(2))
				{
					if (Key2 == false)
					{
						Key2 = true;
						ListView.SelectNext();
						NeedRedraw = true;
					}
				}
				else
				{
					Key2 = false;
				}
				if (ReadGPIOE(3))
				{
					if (Key3 == false)
					{
						Key3 = true;
						ListView.SelectPrev();
						NeedRedraw = true;
					}
				}
				else
				{
					Key3 = false;
				}
				if (ReadGPIOE(4))
				{
					if (Key4 == false)
					{
						Key4 = true;
						StopPlay(PlayerProcess);
						FB.ClearScreen(0);
						NeedRedraw = true;
					}
				}
				else
				{
					Key4 = false;
				}

				if (PlayerProcess != -1 && !IsPlaying(PlayerProcess))
				{
					PlayerProcess = -1;
					FB.ClearScreen(0);
				}
			}
		}

#if !defined(_MSC_VER)
		if (PlayerProcess == -1)
		{
			if (NeedRedraw)
			{
				GUI.Render();
				NeedRedraw = false;
			}
			// FB.RefreshFrontBuffer();
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}
#else
		if (PlayerProcess == -1)
		{
			if (NeedRedraw)
			{
				GUI.Render();
				NeedRedraw = false;
			}
		}
		FB.ProcessMessageNonBlocking();
		if (FB.GetWindowIsDestroyed()) break;
		Sleep(10);
#endif
	}

	return 0;
}


