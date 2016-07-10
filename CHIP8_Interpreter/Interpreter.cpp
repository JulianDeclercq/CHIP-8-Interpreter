#include "Interpreter.h"

#include <iostream>
#include <fstream>

#define DEBUG

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
		unsigned char grayScale = static_cast<unsigned char>(ScaleTo(i, 2048, 255));
		unsigned char grayScaleInverse = 255 - grayScale;
		m_Screen[i] = RgbaToU32(grayScale, grayScaleInverse, grayScale, 255);
	}
}

void Interpreter::ClearScreen()
{
	//TEMPORARY
	TempScreen();
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

void Interpreter::DecreaseTimers() //timers count down at 60hz so might want to
//implement slow down on emulation cycle (60opcodes per sec)
{
	if (m_DelayTimer > 0)
		--m_DelayTimer;

	if (m_SoundTimer > 0)
	{
		--m_SoundTimer;

		if (m_SoundTimer == 1)
			std::cout << "Beep\n";
	}
}

void Interpreter::Cycle()
{
	//Fetch opcode
	//Fetch memory from the location specified by the program counter
	unsigned char o = m_Memory[m_ProgramCounter++]; //increment counter here (1)
	unsigned char p = m_Memory[m_ProgramCounter++]; //increment counter here (2), 2 times incremented => next instruction

	//Opcode is 2 bytes, memory is 1 byte so add them together
	unsigned short opCode;
	opCode = o << 8 | p;

#ifdef DEBUG
	std::cout << std::hex << "Opcode: " << opCode << std::endl;
#endif

	//Decode opcode
	switch (opCode & 0xF000) //read the first four bits of the current opcode (0xF000 in binary is 1111000000000000)
	{
	case 0x0000:
	{
		switch (opCode & 0x000F) //multiple opCodes that start with 0 exist
		{
		case 0x0000: //00E0 	Clears the screen.
			ClearScreen();
			break;
		case 0x000E: //00EE 	Returns from a subroutine.
			break;
		default: std::cout << "Invalid opcode with 0x0000, possibly 0NNN was meant\n";
			break;
		}
	}
	break;

	case 0xA000: //ANNN 	Sets I to the address NNN.
	{
		m_IndexRegister = opCode & 0x0FFF;
	}
	break;

	case 0xB000: //BNNN 	Jumps to the address NNN plus V0.
	{
		m_ProgramCounter = (opCode & 0x0FFF) + m_V[0];
	}
	break;

	case 0xC000: //CXNN 	Sets VX to the result of a bitwise and operation on a random number and NN.
	{
		unsigned char X = (opCode & 0x0F00) >> 8;
		unsigned char NN = opCode & 0x00FF;
		unsigned char randomNr = static_cast<unsigned char>(rand());
		m_V[X] = randomNr & NN;
	}
	break;

	case 0xD000:
	{
		std::cout << "DRAW COMMAND" << std::endl;
	}
	break;

	case 0x1000: //1NNN 	Jumps to address NNN.
	{
	}
	break;
	case 0x2000: //2NNN 	Calls subroutine at NNN.
	{
		m_Stack[m_StackPointer] = m_ProgramCounter; //store current address of the pc
		++m_StackPointer;
		m_ProgramCounter = opCode & 0x0FFF;
		//don't increment pc by 2 because we are calling a subroutine at a specific address
	}
	break;
	case 0x3000: //3XNN 	Skips the next instruction if VX equals NN.
	{
		unsigned char X = (opCode & 0x0F00) >> 8;
		unsigned char NN = opCode & 0x00FF;
		if (m_V[X] == NN) //>> 8 because only first 4 bits were read with main switch
			m_ProgramCounter += 2;
	}
	break;
	case 0x4000: //4XNN 	Skips the next instruction if VX doesn't equal NN.
	{
		unsigned char X = (opCode & 0x0F00) >> 8;
		unsigned char NN = opCode & 0x00FF;
		if (m_V[X] != NN)
			m_ProgramCounter += 2;
	}
	break;
	case 0x5000: //5XY0 	Skips the next instruction if VX equals VY.
	{
		unsigned char X = (opCode & 0x0F00) >> 8;
		unsigned char Y = (opCode & 0x00F0) >> 8;
		if (m_V[X] == m_V[Y])
			m_ProgramCounter += 2;
	}
	break;
	case 0x6000: //6XNN 	Sets VX to NN.
	{
		unsigned char X = (opCode & 0x0F00) >> 8;
		unsigned char NN = (opCode & 0x00FF);
		m_V[X] = NN;
	}
	break;

	case 0x7000: //7XNN 	Adds NN to VX.
	{
		unsigned char X = (opCode & 0x0F00) >> 8;
		unsigned char NN = opCode & 0x00FF;
		m_V[X] += NN;
	}
	break;

	case 0x8000:
	{
		unsigned char X = (opCode & 0x0F00) >> 8;
		unsigned char Y = (opCode & 0x00F0) >> 4;
		switch (opCode & 0x000F)
		{
		case 0x0000: //8XY0 	Sets VX to the value of VY.
			m_V[X] = m_V[Y];
			break;

		case 0x0001: //8XY1 	Sets VX to VX or VY.
			m_V[X] = m_V[X] | m_V[Y];
			break;

		case 0x0002: //8XY2 	Sets VX to VX and VY.
			m_V[X] = m_V[X] & m_V[Y];
			break;

		case 0x0003: //8XY3 	Sets VX to VX xor VY.
			m_V[X] = m_V[X] ^ m_V[Y];
			break;

		case 0x0004: /*8XY4		Adds VY to VX.
					 VF is set to 1 when there's a carry, and to 0 when there isn't.
					 carry means the value is higher than 255 and thus 255 will be added surplus*/
			m_V[0xF] = (m_V[Y] > (0xFF - m_V[X])) ? 1 : 0;
			m_V[X] += m_V[Y];
			break;

		case 0x0005: //8XY5 	VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
			m_V[0xF] = (m_V[Y] > m_V[X]) ? 0 : 1;
			m_V[X] -= m_V[Y];
			break;

		case 0x0006: //8XY6 	Shifts VX right by one. VF is set to the value of the least significant bit of VX before the shift.[2]
			m_V[0xF] = m_V[X] & 1; //least significant bit
			m_V[X] = m_V[X] >> 1;
			break;

		case 0x0007: //8XY7 	Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
			m_V[0xF] = (m_V[Y] > m_V[X]) ? 0 : 1;
			m_V[X] = m_V[Y] - m_V[X];
			break;

		case 0x000E: //8XYE 	Shifts VX left by one. VF is set to the value of the most significant bit of VX before the shift.
			m_V[0xF] = (m_V[X] >> 7) & 1; // most significant bit
			m_V[X] = m_V[X] << 1;
			break;

		default: std::cout << "Invalid opcode in case 0x8000 \n";
			break;
		}
	}
	break;

	case 0x9000: //9XY0 	Skips the next instruction if VX doesn't equal VY.
	{
		unsigned char X = (opCode & 0x0F00) >> 8;
		unsigned char Y = (opCode & 0x00F0) >> 4;
		if (m_V[X] != m_V[Y])
			m_ProgramCounter += 2;
	}
	break;

	default: std::cout << "Invalid main opcode reached\n";
		break;
	}

	//Execute opcode

	//Update timers
	DecreaseTimers();
}