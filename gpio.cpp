#include "gpio.hpp"

#include <cstdio>
#include <cstdlib>
bool GPIO_PeriphType::ReadBit(int Port) const
{
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
	char cmd[1024];

	snprintf(cmd, sizeof(cmd), "devmem 0x%08x 32 0x%08x", reinterpret_cast<size_t>(Ptr), Data);

	FILE *fp = popen(cmd, "r");
	if (fp == NULL)
	{
		fprintf(stderr, "%s: popen(%s) fail!\n", __func__, cmd);
	}
	fclose(fp);
#endif
}

uint32_t GPIO_PeriphType::ReadPeriph(const volatile uint32_t* Ptr)
{
#if !defined(_MSC_VER)
	uint32_t value = 0;
	char cmd[1024];
	char buf[32];

	snprintf(cmd, sizeof(cmd), "devmem 0x%08x", reinterpret_cast<size_t>(Ptr));

	FILE* fp = popen(cmd, "r");
	if (fp == NULL)
	{
		fprintf(stderr, "%s: popen(%s) fail!\n", __func__, cmd);
	}
	else
	{
		fread(buf, sizeof(char), sizeof(buf), fp);
		sscanf(buf, "0x%x", &value);
		pclose(fp);
	}
	return value;
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
	return GPIO_Periph[GPIO_E].ReadBit(Port);
}

bool ReadGPIOF(int Port)
{
	GPIO_Periph[GPIO_F].SetModeIn(Port);
	return GPIO_Periph[GPIO_F].ReadBit(Port);
}
