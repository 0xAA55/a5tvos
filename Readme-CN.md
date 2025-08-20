# A5TVOS 小电视 OS 界面，基于 F1C200S

## 语言 Language

简体中文|[Chinglish](Readme.md)

## 简述
使用全败的 F1C200S CPU 工作在 `800 MHz` 然后控制 `FFmpeg` 进程进行 MJPEG 视频软解播放，达到 30 fps。使用特制的 `tinyplay` 从 `stdin` 读取 `WAV-PCM` 音频 `S16LE` 格式进行音频播放。使用 GPIOE 1、2、3、4 读取四个按键操作。

使用 C++ 以写 `/dev/fb0` 的方式直接绘制播放列表，然后启动 `FFmpeg` + `tinyplay` 以管道通讯方式实现视频解码后，视频走 `/dev/fb0` 播放，音频走 `tinyalsa` 播放。

## 硬件需求
* CPU 主频需要达到 `800 MHz`，配置 F1C200S ROM 的 Buildroot 里面的 DTS 可以设置此频率。
	* 不一定要 F1C200S，之所以有这个需求是因为用不了 F1C200S 的 JPEG 硬解，只好超频用 FFmpeg 软解。
* 有 40pin RGB 接口，接 `480x272` 分辨率的屏幕，其中对 `/dev/fb0` 的读写要能直接体现到此屏幕。
* 音频接口：`tinyalsa`
* GPIO 外设地址：`0x01C20800`，寄存器布局：
	```cpp
	struct GPIO_PeriphType
	{
		volatile uint32_t CFG0;
		volatile uint32_t CFG1;
		volatile uint32_t CFG2;
		volatile uint32_t CFG3;
		volatile uint32_t DATA;
		volatile uint32_t DRV0;
		volatile uint32_t DRV1;
		volatile uint32_t PUL0;
		volatile uint32_t PUL1;

		/* 成员方法 */

		bool ReadBit(int Port) const;
		void WriteBit(int Port, bool Value);

		void SetModeIn(int Port);
		void SetModeOut(int Port);
		void SetModeDisabled(int Port);

	protected:
		void SetMode(int Port, uint32_t Mode);

		static void WritePeriph(volatile uint32_t* Ptr, uint32_t Data);
		static uint32_t ReadPeriph(const volatile uint32_t* Ptr);
	};
	```
	读写外设、艹 GPIO 的方式为：
	```cpp
	void GPIO_PeriphType::SetMode(int Port, uint32_t Mode)
	{
		int Shift;
		if (Port < 8)
		{
			Shift = Port * 4;
			WritePeriph(&CFG0, ReadPeriph(&CFG0) & (~(7 << Shift)) | (Mode << Shift));
		}
		else if (Port < 16)
		{
			Shift = (Port - 8) * 4;
			WritePeriph(&CFG1, ReadPeriph(&CFG1) & (~(7 << Shift)) | (Mode << Shift));
		}
		else if (Port < 24)
		{
			Shift = (Port - 16) * 4;
			WritePeriph(&CFG2, ReadPeriph(&CFG2) & (~(7 << Shift)) | (Mode << Shift));
		}
		else 
		{
			Shift = (Port - 24) * 4;
			WritePeriph(&CFG3, ReadPeriph(&CFG3) & (~(7 << Shift)) | (Mode << Shift));
		}
	}
	```
	其中：`WritePeriph()` 的行为是使用 `mmap()` 按照内存页的颗粒度映射外设地址，然后计算偏移量后直接艹外设寄存器，再 `munmap()` 应用内存页的改动；如果不能用 `mmap()` 则会使用 `popen()` 调用 `devmem` 进行内存操作，并通过分析 `devmem` 的标准输出来读取内存内容。
* 媒体文件存储于 SD 卡，路径为 `/dev/mmcblk0p1`，可用 `mount` 命令挂载至 `/mnt/sdcard`。

## 软件需求
* 要能访问到 `/dev/fb0` 实现写屏。
* 要能访问到 `/dev/mem` 使用 `mmap()` 可直接读写外设内存，如果不能使用 `mmap()` 读写这块内存，则要能提供 `devmem` 可执行文件用于执行相同的操作。
* 要能使用 `mount()`、`umount()` 函数。
* 媒体文件从 `/mnt/sdcard` 可以枚举并访问到。

## 调试环境

### 模拟调试环境
* 使用 VS2022 调试整体的程序逻辑。使用条件编译，在 Windows 上使用窗口模拟界面自绘和视频播放的效果。

### 实机调试环境
* F1C200S 的命令行走 USB 虚拟 UART 与主机通讯。
* 源码使用条件编译，在 F1C200S 上使用 `/dev/fb0` 绘制界面并使用 `FFmpeg` + `tinyalsa` 进行音视频播放。

## 编译
* Windows 下，解压 `tools.zip` 到当前路径。双击 `build.bat`，其调用 `wsl` 执行 `build.sh` 进行编译。
* Linux 下，具备能交叉编译到 `-march=armv5t` 的 CPU 的 gcc 即可。
