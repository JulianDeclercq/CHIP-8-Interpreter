#include "Interpreter.h"

#include <iostream>
#include <fstream>

Interpreter::Interpreter()
{
	TempScreen();
}

Interpreter::~Interpreter()
{
}

inline int ScaleTo(int value, int from, int to)
{
	float fraction = static_cast<float>(to) / static_cast<float>(from);
	return static_cast<int>(fraction * value);
}

unsigned int RgbaToU32(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	unsigned int u32 = r << 24 | g << 16 | b << 8 | a;
	return u32;
}

void Interpreter::TempScreen()
{
	for (int i = 0; i < 2048; ++i)
	{
		int grayScale = ScaleTo(i, 2048, 255);
		int grayScaleInverse = 255 - grayScale;
		m_Screen[i] = RgbaToU32(grayScale, grayScaleInverse, grayScale, 255);
	}
}

void Interpreter::Initialize()
{
	for (int i = 0; i < REGISTER_COUNT; ++i)
		m_V[i] = 0;

	for (int i = 0; i < PIXEL_COUNT; ++i)
		m_Screen[i] = 0;

	for (int i = 0; i < STACK_COUNT; ++i)
		m_Stack[i] = 0;

	for (int i = 0; i < KEYPAD_COUNT; ++i)
		m_Keypad[i] = 0;

	/*Loading the fontset to memory.
	Wikipedia: In modern CHIP-8 implementations,
	where the interpreter is running natively outside the 4K memory space,
	there is no need for any of the lower 512 bytes memory space to be used,
	but it is common to store font data in those lower 512 bytes (0x000-0x200)*/
	for (int i = 0; i < FONTSET_SIZE; ++i)
		m_Memory[i] = m_Fontset[i];

	//Fill opcodes
	InitialiseOpcodes();
}

void Interpreter::LoadRom(const std::string& path)
{
	std::ifstream Rom;
	Rom.open(path, std::ios_base::binary | std::ios_base::ate);
	if (Rom.fail())
	{
		std::cout << "Failed to load Rom with path " << path << std::endl;
		return;
	}
	const int fileSize = static_cast<int>(Rom.tellg());
	Rom.seekg(0); //go back to the beginning of the file

	Rom.read(reinterpret_cast<char*>(m_Memory + 512), fileSize);
}

unsigned int* Interpreter::GetScreen()
{
	return m_Screen;
}

unsigned short Interpreter::FetchOpCode()
{
	//Fetch memory from the location specified by the program counter
	unsigned char o = m_Memory[m_ProgramCounter];
	unsigned char p = m_Memory[m_ProgramCounter + 1];

	//Opcode is 2 bytes, memory is 1 byte so add them together
	unsigned short opCode = o << 8 | p;
	return opCode;
}

void Interpreter::Cycle()
{
	//Fetch opcode
	FetchOpCode();
	//Decode opcode
	//Execute opcode

	//Update timers
}