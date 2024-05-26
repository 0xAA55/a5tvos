#include "gpio.hpp"

bool GPIO_PeriphType::ReadBit(int Port) const
{
	return (DATA & (1 << Port)) ? true : false;
}

void GPIO_PeriphType::WriteBit(int Port, bool Value)
{
	uint32_t Bit = 1 << Port;
	DATA = (DATA & (~Bit)) | (Value ? Bit: 0);
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
		CFG0 = CFG0 & (~(7 << Shift)) | (Mode << Shift);
	}
	else if (Port < 16)
	{
		Shift = (Port - 8) * 4;
		CFG1 = CFG1 & (~(7 << Shift)) | (Mode << Shift);
	}
	else if (Port < 24)
	{
		Shift = (Port - 16) * 4;
		CFG2 = CFG2 & (~(7 << Shift)) | (Mode << Shift);
	}
	else 
	{
		Shift = (Port - 24) * 4;
		CFG3 = CFG3 & (~(7 << Shift)) | (Mode << Shift);
	}
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
	GPIO_Periph[GPIO_A].ReadBit(Port);
}

bool ReadGPIOB(int Port)
{
	GPIO_Periph[GPIO_B].SetModeIn(Port);
	GPIO_Periph[GPIO_B].ReadBit(Port);
}

bool ReadGPIOC(int Port)
{
	GPIO_Periph[GPIO_C].SetModeIn(Port);
	GPIO_Periph[GPIO_C].ReadBit(Port);
}

bool ReadGPIOD(int Port)
{
	GPIO_Periph[GPIO_D].SetModeIn(Port);
	GPIO_Periph[GPIO_D].ReadBit(Port);
}

bool ReadGPIOE(int Port)
{
	GPIO_Periph[GPIO_E].SetModeIn(Port);
	GPIO_Periph[GPIO_E].ReadBit(Port);
}

bool ReadGPIOF(int Port)
{
	GPIO_Periph[GPIO_F].SetModeIn(Port);
	GPIO_Periph[GPIO_F].ReadBit(Port);
}
