#include "gpio.hpp"

#include <cstdio>
#include <cstdlib>

#if defined(_MSC_VER)
#include <Windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
int MemFD = open("/dev/mem", O_RDWR | O_SYNC);
#endif

bool GPIO_PeriphType::ReadBit(int Port) const
{
#ifdef _MSC_VER
	if (this == &GPIO_Periph[GPIO_E])
	{
		switch (Port)
		{
		case 1: return bool(GetAsyncKeyState('V'));
		case 2: return bool(GetAsyncKeyState('C'));
		case 3: return bool(GetAsyncKeyState('X'));
		case 4: return bool(GetAsyncKeyState('Z'));
		}
	}
#endif
	return (ReadPeriph(&DATA) & (1 << Port)) ? true : false;
}

void GPIO_PeriphType::WriteBit(int Port, bool Value)
{
	uint32_t Bit = 1 << Port;
	WritePeriph(&DATA, (ReadPeriph(&DATA) & (~Bit)) | (Value ? Bit : 0));
}

void GPIO_PeriphType::SetModeIn(int Port)
{
	SetMode(Port, 0);
}

void GPIO_PeriphType::SetModeOut(int Port)
{
	SetMode(Port, 1);
}

void GPIO_PeriphType::SetModeDisabled(int Port)
{
	SetMode(Port, 7);
}

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

void GPIO_PeriphType::WritePeriph(volatile uint32_t* Ptr, uint32_t Data)
{
#if !defined(_MSC_VER)
	const uint32_t MapSize = getpagesize();
	const uint32_t MapMask = MapSize - 1;
	volatile uint32_t* VirtAddr = nullptr;
	uint32_t Addr = uint32_t(reinterpret_cast<size_t>(Ptr));
	uint32_t* MapPtr = nullptr;

	if (MemFD == -1) goto NotAbleToMap;

	MapPtr = (uint32_t*)mmap(0, MapSize, PROT_READ | PROT_WRITE, MAP_SHARED, MemFD, Addr & ~MapMask);
	if (MapPtr == (uint32_t*)-1) goto NotAbleToMap;

	VirtAddr = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<char *>(MapPtr) + (Addr & MapMask));
	*VirtAddr = Data;

	munmap(MapPtr, MapSize);
	return;

NotAbleToMap:
	char cmd[1024];
	snprintf(cmd, sizeof(cmd), "devmem 0x%08x 32 0x%08x", uint32_t(reinterpret_cast<size_t>(Ptr)), Data);
	system(cmd);
#endif
}

uint32_t GPIO_PeriphType::ReadPeriph(const volatile uint32_t* Ptr)
{
#if !defined(_MSC_VER)
	const uint32_t MapSize = getpagesize();
	const uint32_t MapMask = MapSize - 1;
	volatile uint32_t* VirtAddr = nullptr;
	uint32_t Addr = uint32_t(reinterpret_cast<size_t>(Ptr));
	uint32_t* MapPtr = nullptr;
	uint32_t Data = 0;

	if (MemFD == -1) goto NotAbleToMap;

	MapPtr = (uint32_t*)mmap(0, MapSize, PROT_READ, MAP_SHARED, MemFD, Addr & ~MapMask);
	if (MapPtr == (uint32_t*)-1) goto NotAbleToMap;

	VirtAddr = reinterpret_cast<volatile uint32_t*>(reinterpret_cast<char*>(MapPtr) + (Addr & MapMask));
	Data = *VirtAddr;

	munmap(MapPtr, MapSize);
	return Data;

NotAbleToMap:
	char cmd[1024];
	snprintf(cmd, sizeof(cmd), "devmem 0x%08x", uint32_t(reinterpret_cast<size_t>(Ptr)));
	FILE* fp = popen(cmd, "r");
	if (fp == NULL)
	{
		fprintf(stderr, "%s: popen(%s) fail!\n", __func__, cmd);
	}
	else
	{
		char buf[32];
		uint32_t value = 0;
		fread(buf, sizeof(char), sizeof(buf), fp);
		sscanf(buf, "0x%x", &value);
		pclose(fp);
		return value;
	}
#endif
	return 0;
}

void WriteGPIOA(int Port, bool Value)
{
	GPIO_Periph[GPIO_A].SetModeOut(Port);
	GPIO_Periph[GPIO_A].WriteBit(Port, Value);
}

void WriteGPIOB(int Port, bool Value)
{
	GPIO_Periph[GPIO_B].SetModeOut(Port);
	GPIO_Periph[GPIO_B].WriteBit(Port, Value);
}

void WriteGPIOC(int Port, bool Value)
{
	GPIO_Periph[GPIO_C].SetModeOut(Port);
	GPIO_Periph[GPIO_C].WriteBit(Port, Value);
}

void WriteGPIOD(int Port, bool Value)
{
	GPIO_Periph[GPIO_D].SetModeOut(Port);
	GPIO_Periph[GPIO_D].WriteBit(Port, Value);
}

void WriteGPIOE(int Port, bool Value)
{
	GPIO_Periph[GPIO_E].SetModeOut(Port);
	GPIO_Periph[GPIO_E].WriteBit(Port, Value);
}

void WriteGPIOF(int Port, bool Value)
{
	GPIO_Periph[GPIO_F].SetModeOut(Port);
	GPIO_Periph[GPIO_F].WriteBit(Port, Value);
}

bool ReadGPIOA(int Port)
{
	GPIO_Periph[GPIO_A].SetModeIn(Port);
	return GPIO_Periph[GPIO_A].ReadBit(Port);
}

bool ReadGPIOB(int Port)
{
	GPIO_Periph[GPIO_B].SetModeIn(Port);
	return GPIO_Periph[GPIO_B].ReadBit(Port);
}

bool ReadGPIOC(int Port)
{
	GPIO_Periph[GPIO_C].SetModeIn(Port);
	return GPIO_Periph[GPIO_C].ReadBit(Port);
}

bool ReadGPIOD(int Port)
{
	GPIO_Periph[GPIO_D].SetModeIn(Port);
	return GPIO_Periph[GPIO_D].ReadBit(Port);
}

bool ReadGPIOE(int Port)
{
	GPIO_Periph[GPIO_E].SetModeIn(Port);
#if !defined(_MSC_VER)
	return GPIO_Periph[GPIO_E].ReadBit(Port);
#else
	GPIO_Periph[GPIO_E].ReadBit(Port);
	switch (Port)
	{
	case 1: return bool(GetAsyncKeyState('V'));
	case 2: return bool(GetAsyncKeyState('C'));
	case 3: return bool(GetAsyncKeyState('X'));
	case 4: return bool(GetAsyncKeyState('Z'));
	}
	return false;
#endif
}

bool ReadGPIOF(int Port)
{
	GPIO_Periph[GPIO_F].SetModeIn(Port);
	return GPIO_Periph[GPIO_F].ReadBit(Port);
}
