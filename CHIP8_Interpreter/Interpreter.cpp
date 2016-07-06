#include "Interpreter.h"

#include <iostream>

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

unsigned int* Interpreter::GetScreen()
{
	return m_Screen;
}