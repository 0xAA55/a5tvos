# A5TVOS Tiny TV OS Interface (Based on F1C200S)

## 语言 Language

[简体中文](Readme-CN.md)|Chinglish

## Intro
Utilizing the All-Loser F1C200S CPU operating at `800 MHz` to control the `FFmpeg` process for MJPEG video software decoding, achieving `30 FPS`. A custom `tinyplay` reads `WAV-PCM` audio in `S16LE` format from `stdin` for audio playback. Key inputs are detected via GPIOE pins 1, 2, 3, and 4.

The system is developed in **C++**, directly writing to `/dev/fb0` for rendering the playlist. `FFmpeg` and `tinyplay` are launched with pipeline communication: video output is streamed to `/dev/fb0`, while audio is routed to `tinyalsa`.

## Hardware Requirements
* CPU frequency must reach `800 MHz`. This can be configured in the **Device Tree Source (DTS)** of the F1C200S Buildroot ROM.
	* *Note: F1C200S is not mandatory. This requirement stems from the inability to use its JPEG hardware decoding, necessitating software decoding via FFmpeg at higher clock speeds.*
* 40-pin RGB interface for a `480x272` resolution screen. Reads/writes to `/dev/fb0` must directly output to this display.
* Audio interface: `tinyalsa`
* GPIO peripheral address: `0x01C20800` (register layout):
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

		/* Member methods */

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
	GPIO manipulation logic:
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
	* `WritePeriph()` behavior: Uses `mmap()` to map peripheral addresses at memory-page granularity. Directly modifies peripheral registers after calculating offsets, followed by `munmap()` to apply changes. If `mmap()` is unavailable, falls back to using `popen()` to call `devmem` for memory operations, parsing its `stdout` to read memory content.
* Media storage: SD card at `/dev/mmcblk0p1`, mounted to `/mnt/sdcard` via `mount`.

## Software Requirements
* Access to `/dev/fb0` for display output.
* Access to `/dev/mem` to enable `mmap()` for direct peripheral memory reads/writes. If unavailable, the `devmem` executable must be provided as a fallback.
* Support for `mount()` and `umount()` functions.
* Media files must be enumerable and accessible from `/mnt/sdcard`.

## Debugging Environments

### Simulated Environment
* Uses **Visual Studio 2022** to debug overall program logic. Conditional compilation (`#ifdef`) simulates interface rendering and video playback on Windows.

### Physical Device Environment
* F1C200S command line communicates with the host via USB Virtual UART for debug usages.
* Source code uses conditional compilation to deploy on F1C200S:
	* Renders interface via `/dev/fb0`.
	* Uses `FFmpeg` + `tinyalsa` for audio/video playback.
