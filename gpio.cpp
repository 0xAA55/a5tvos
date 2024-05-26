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
