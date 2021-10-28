#pragma once

#include <string>
#include <Windows.h>
#include <DirectXMath.h>

class Debug
{
public:
	Debug() {};
	static void Print(std::string output);
	static void Print(int output);
	static void Print(float output);
	static void Print(DirectX::XMFLOAT3 output);
};