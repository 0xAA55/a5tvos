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
