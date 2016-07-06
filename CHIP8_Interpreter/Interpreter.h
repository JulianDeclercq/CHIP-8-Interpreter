#pragma once

using opcode = unsigned short;
using byte = unsigned char;

class Interpreter
{
public:
	Interpreter();
	~Interpreter();

	void TempScreen();
	unsigned int* GetScreen();

private:

	/* Systems memory map (total system memory is 4096 bytes)
	0x000-0x1FF - Chip 8 interpreter (contains font set in emu)
	0x050-0x0A0 - Used for the built in 4x5 pixel font set (0-F)
	0x200-0xFFF - Program ROM and work RAM*/
	byte m_Memory[4096];

	byte m_V[16]; //there are 16 8-bit registers

	//both can have a value from 0x000 to 0xFFF
	unsigned short I; //index register
	unsigned short pc; //program counter

	//screen has 2048 pixels (64 x 32)
	unsigned int m_Screen[64 * 32];

	//2 timers
	unsigned char m_DelayTimer;
	unsigned char m_SoundTimer; //The systems buzzer sounds whenever the sound timer reaches zero

	//Stack and stack pointer
	unsigned short m_Stack[16];
	unsigned short m_StackPointer;

	//hex based keypad
	unsigned char m_Keypad[16];
};
