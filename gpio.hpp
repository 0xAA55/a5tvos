#pragma once

#include <cstdint>

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

	bool ReadBit(int Port) const;
	void WriteBit(int Port, bool Value);

	void SetModeIn(int Port);
	void SetModeOut(int Port);
	void SetModeDisabled(int Port);

protected:
	void SetMode(int Port, uint32_t Mode);
};

GPIO_PeriphType * const GPIO_Periph = reinterpret_cast<GPIO_PeriphType * const>(0x01C20800);

enum GPIO_GroupEnumType
{
	GPIO_A = 0,
	GPIO_B = 0,
	GPIO_C = 0,
	GPIO_D = 0,
	GPIO_E = 0,
	GPIO_F = 0,
};

void WriteGPIOA(int Port, bool Value);
void WriteGPIOB(int Port, bool Value);
void WriteGPIOC(int Port, bool Value);
void WriteGPIOD(int Port, bool Value);
void WriteGPIOE(int Port, bool Value);
void WriteGPIOF(int Port, bool Value);

bool ReadGPIOA(int Port);
bool ReadGPIOB(int Port);
bool ReadGPIOC(int Port);
bool ReadGPIOD(int Port);
bool ReadGPIOE(int Port);
bool ReadGPIOF(int Port);
