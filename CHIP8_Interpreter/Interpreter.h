#pragma once

#include <string>

class Interpreter
{
public:
	Interpreter();
	~Interpreter();

	void TempScreen();
	void LoadRom(const std::string& path);

	void Initialize();
	unsigned int* GetScreen();

private:

	/* Systems memory map (total system memory is 4096 bytes)
	0x000-0x1FF - Chip 8 interpreter (contains font set in emu)
	0x050-0x0A0 - Used for the built in 4x5 pixel font set (0-F)
	0x200-0xFFF - Program ROM and work RAM*/
	unsigned char m_Memory[4096];

	//Current opcode
	unsigned short m_Opcode = 0;

	static const int REGISTER_COUNT = 16;
	unsigned char m_V[REGISTER_COUNT]; //16 8-bit registers named from V0 to VF

	//both can have a value from 0x000 to 0xFFF
	unsigned short m_IndexRegister = 0; //index register
	unsigned short m_ProgramCounter = 0x200; //program counter, starts at 0x200

	//screen has 2048 pixels (64 x 32)
	static const int PIXEL_COUNT = 64 * 32;
	unsigned int m_Screen[PIXEL_COUNT];

	//2 timers
	unsigned char m_DelayTimer = 0; //should be 60?
	unsigned char m_SoundTimer = 0; //The systems buzzer sounds whenever the sound timer reaches zero

	//Stack and stack pointer
	static const int STACK_COUNT = 16;
	unsigned short m_Stack[STACK_COUNT];
	unsigned short m_StackPointer = 0;

	//hex based keypad
	static const int KEYPAD_COUNT = 16;
	unsigned char m_Keypad[KEYPAD_COUNT];

	//Fontset
	static const int FONTSET_SIZE = 80;
	const unsigned char m_Fontset[FONTSET_SIZE] =
	{
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	};
};
