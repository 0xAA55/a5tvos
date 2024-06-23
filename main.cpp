
#include "graphics.hpp"
#include "gui.hpp"
#include "gpio.hpp"

#if !defined(_MSC_VER)
#include <sys/mount.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <csignal>
#include <iostream>
#include <filesystem>

#include <chrono>
#include <thread>
#include <set>

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

void DbgPrintf(const char* format, ...)
{
	va_list ap;
	va_start(ap, format);
	vprintf(format, ap);
	va_end(ap);
}

#if defined(_MSC_VER)
using pid_t = int;
#endif

#if !defined(_MSC_VER)
int RunPipedCommand(const char* CmdPipeWriter, const char* CmdPipeReader, int& PidVideo, int& PidAudio)
{
	int pipefd[2];
	PidVideo = -1;
	PidAudio = -1;
	if (pipe(pipefd) == -1)
	{
		perror("pipe");
		return -1;
	}

	PidVideo = fork();
	if (PidVideo < 0) goto FailExit;

	if (PidVideo == 0)
	{
		close(pipefd[0]);
		dup2(pipefd[1], STDOUT_FILENO);

		execl("/bin/sh", "sh", "-c", CmdPipeWriter, NULL);
		perror("execl()");
		exit(-1);
	}

	PidAudio = fork();
	if (PidAudio < 0) goto FailExit;

	if (PidAudio == 0)
	{
		dup2(pipefd[0], STDIN_FILENO);
		close(pipefd[1]);

		execl("/bin/sh", "sh", "-c", CmdPipeReader, NULL);
		perror("execl()");
		exit(-1);
	}

	return 0;
FailExit:
	close(pipefd[0]);
	close(pipefd[1]);
	if (PidVideo > 0) exit(-1);
	if (PidAudio > 0) exit(-1);
	return -1;
}
#else
pid_t RunCommand(const char* command)
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
		return -1;
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

int PlayVideo(const std::string& VideoFile, int& PidVideo, int& PidAudio)
{
	char buf[4096];

#ifndef _MSC_VER
	system("tinymix set 1 48");
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
	snprintf(buf, sizeof buf, "ffmpeg -hide_banner -i \"%s\" -an -pix_fmt bgra -f fbdev /dev/fb0 -vn -f wav pipe:1 -ar 44100 -ac 1 | tinyplay stdin -r 44100 -c 1", VideoFile.c_str());
	DbgPrintf("%s\n", buf);
	snprintf(buf, sizeof buf, "ffmpeg -hide_banner -i \"%s\" -an -pix_fmt bgra -f fbdev /dev/fb0 -vn -f wav pipe:1 -ar 44100 -ac 1", VideoFile.c_str());
	RunPipedCommand(buf, "tinyplay stdin -r 44100 -c 1", PidVideo, PidAudio);
#else
	snprintf(buf, sizeof buf, "ffplay %s", VideoFile.c_str());
	DbgPrintf("%s\n", buf);
	PidVideo = RunCommand(buf);
	PidAudio = PidVideo;
#endif
	DbgPrintf("Video Player PID: %d\n", PidVideo);
	DbgPrintf("Audio Player PID: %d\n", PidAudio);
	return 0;
}

void StopPlay(pid_t& VideoPlayerPID, pid_t& AudioPlayerPID)
{
	if (VideoPlayerPID == -1 && AudioPlayerPID == -1) return;

#ifndef _MSC_VER
	if (VideoPlayerPID != -1)
	{
		DbgPrintf("Killing video player PID: %u\n", VideoPlayerPID);
		kill(VideoPlayerPID, SIGINT);
		VideoPlayerPID = -1;
	}
	if (AudioPlayerPID != -1)
	{
		DbgPrintf("Killing audio player PID: %u\n", AudioPlayerPID);
		kill(AudioPlayerPID, SIGINT);
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		kill(AudioPlayerPID, SIGTERM);
		AudioPlayerPID = -1;
	}
#else
	const auto player = OpenProcess(PROCESS_TERMINATE, false, DWORD(VideoPlayerPID));
	TerminateProcess(player, 1);
	CloseHandle(player);
	VideoPlayerPID = -1;
	AudioPlayerPID = -1;
#endif
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
	int wstatus;
	auto wpid = waitpid(pid, &wstatus, WNOHANG);
	if (wpid == pid)
	{
		if (WIFEXITED(wstatus)) return false;
		printf("Exit code: %d\n", WEXITSTATUS(wstatus));
	}
	return true;
#endif
}

int main(int argc, char** argv, char** envp)
{
	const int ResoW = 480;
	const int ResoH = 272;

	bool Mounted = false;

	WriteGPIOE(0, true);
	GPIO_Periph[GPIO_E].SetModeIn(1);
	GPIO_Periph[GPIO_E].SetModeIn(2);
	GPIO_Periph[GPIO_E].SetModeIn(3);
	GPIO_Periph[GPIO_E].SetModeIn(4);

	bool NeedRedraw = true;

	pid_t VideoPlayerPID = -1;
	pid_t AudioPlayerPID = -1;

#if !defined(_MSC_VER)
	auto FB = Graphics(ResoW, ResoH, false);
	FB.SetBackBufferMode();
#else
	auto FB = MyTestApp(false);
#endif
	FB.ClearScreen(0);
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

	do
	{
		auto Sub = std::make_shared<UIElementLabel>(FB, "Title");
		GUI.InsertElement(Sub);
		Sub->XMargin = 0;
		Sub->YMargin = 0;
		Sub->XBorder = 0;
		Sub->YBorder = 1;
		Sub->XPadding = 2;
		Sub->YPadding = 2;
		Sub->BorderColor = 0xFFFFFFFF;
		Sub->FillColor = 0xFF000000;
		Sub->FontColor = 0xFFFFFFFF;
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
		Sub->FillColor = 0xFF000000;
		Sub->FontColor = 0xFFFFFFFF;
		Prompt->ExpandToParentX = true;
		Prompt->ExpandToParentY = true;
		Prompt->LineBreak = false;
		Prompt->Transparent = true;
		Prompt->Alignment = AlignmentType::CenterCenter;
		Prompt->SetCaption("请插入 SD 卡。可在播放时随时拔出 SD 卡。\n");

		NeedRedraw = true;
	} while (false);

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
				StopPlay(VideoPlayerPID, AudioPlayerPID);

#if !defined(_MSC_VER)
				umount2("/mnt/sdcard", MNT_FORCE);
#endif
				Mounted = false;

				GUI.ClearElements();
				FB.ClearScreen(0);

				auto Sub = std::make_shared<UIElementLabel>(FB, "Title");
				GUI.InsertElement(Sub);
				Sub->XMargin = 0;
				Sub->YMargin = 0;
				Sub->XBorder = 0;
				Sub->YBorder = 1;
				Sub->XPadding = 2;
				Sub->YPadding = 2;
				Sub->BorderColor = 0xFFFFFFFF;
				Sub->FillColor = 0xFF000000;
				Sub->FontColor = 0xFFFFFFFF;
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
				Sub->FillColor = 0xFF000000;
				Sub->FontColor = 0xFFFFFFFF;
				Prompt->ExpandToParentX = true;
				Prompt->ExpandToParentY = true;
				Prompt->LineBreak = false;
				Prompt->Transparent = true;
				Prompt->Alignment = AlignmentType::CenterCenter;
				Prompt->SetCaption("请插入 SD 卡。可在播放时随时拔出 SD 卡。\n");

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
					FB.ClearScreen(0);
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
					Sub->FillColor = 0xFF000000;
					Sub->FontColor = 0xFFFFFFFF;
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

					auto Files = std::set<std::string>();
					for (auto& directory : std::filesystem::directory_iterator(media_path))
					{
						// 跳过文件夹
						if (directory.is_directory()) continue;

						auto p = directory.path();
						auto FileNameString = p.filename().string();
						Files.insert(FileNameString);
					}
					for (auto& filename : Files)
					{
						auto ListItem = std::make_shared<UIElementListItem>(FB, filename);
						ListView->AddItem(filename, filename);
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
				if (VideoPlayerPID == -1 && AudioPlayerPID == -1)
				{
					if (GPIO_Periph[GPIO_E].ReadBit(1))
					{
						auto VideoFile = (SDCardPath / ListView.GetSelectedItem().GetCaption()).string();
						StopPlay(VideoPlayerPID, AudioPlayerPID);
						FB.ClearScreen(0);
						FB.RefreshFrontBuffer();
						PlayVideo(VideoFile, VideoPlayerPID, AudioPlayerPID);
					}
					if (GPIO_Periph[GPIO_E].ReadBit(2))
					{
						ListView.SelectNext();
						NeedRedraw = true;
					}
					if (GPIO_Periph[GPIO_E].ReadBit(3))
					{
						ListView.SelectPrev();
						NeedRedraw = true;
					}
				}
				else
				{
					if (GPIO_Periph[GPIO_E].ReadBit(2))
					{
						StopPlay(VideoPlayerPID, AudioPlayerPID);
						ListView.SelectNext();
						auto VideoFile = (SDCardPath / ListView.GetSelectedItem().GetCaption()).string();
						PlayVideo(VideoFile, VideoPlayerPID, AudioPlayerPID);
					}
					if (GPIO_Periph[GPIO_E].ReadBit(3))
					{
						StopPlay(VideoPlayerPID, AudioPlayerPID);
						ListView.SelectPrev();
						auto VideoFile = (SDCardPath / ListView.GetSelectedItem().GetCaption()).string();
						PlayVideo(VideoFile, VideoPlayerPID, AudioPlayerPID);
					}
					if (GPIO_Periph[GPIO_E].ReadBit(4))
					{
						StopPlay(VideoPlayerPID, AudioPlayerPID);
						std::this_thread::sleep_for(std::chrono::milliseconds(100));
						FB.ClearScreen(0);
						NeedRedraw = true;
					}
				}
			}

			if ((VideoPlayerPID != -1 || AudioPlayerPID != -1) && (!IsPlaying(VideoPlayerPID) || !IsPlaying(AudioPlayerPID)))
			{
				StopPlay(VideoPlayerPID, AudioPlayerPID);
				FB.ClearScreen(0);
				NeedRedraw = true;
			}
		}

		if (VideoPlayerPID == -1 && AudioPlayerPID == -1)
		{
			if (NeedRedraw)
			{
				GUI.Render();
				NeedRedraw = false;
			}
#if !defined(_MSC_VER)
			FB.RefreshFrontBuffer();
#endif
			if (!Mounted)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			}
			else
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
			}
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(2000));
		}
#if defined(_MSC_VER)
		FB.RefreshFB();
		FB.ProcessMessageNonBlocking();
		if (FB.GetWindowIsDestroyed()) break;
		Sleep(10);
#endif
	}

	return 0;
}


