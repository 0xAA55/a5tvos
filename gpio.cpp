#include "gpio.hpp"

bool GPIO_PeriphType::ReadBit(int Port) const
{
	return DATA & (1 << Port);
}

void GPIO_PeriphType::WriteBit(int Port)
{
	uint32_t Bit = 1 << Port;
	DATA = (DATA & (~Bit)) | Bit;
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
