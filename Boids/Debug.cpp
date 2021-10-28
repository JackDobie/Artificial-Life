#include "Debug.h"

void Debug::Print(std::string output)
{
	OutputDebugStringA((output + "\n").c_str());
}
void Debug::Print(int output)
{
	char sz[1024] = { 0 };
	sprintf_s(sz, "%d \n", output);
	OutputDebugStringA(sz);
}
void Debug::Print(float output)
{
	char sz[1024] = { 0 };
	sprintf_s(sz, "%f \n", output);
	OutputDebugStringA(sz);
}
void Debug::Print(DirectX::XMFLOAT3 output)
{
	std::string x = std::to_string(output.x);
	std::string y = std::to_string(output.y);
	std::string z = std::to_string(output.z);
	std::string out = "X:" + x + ", Y:" + y + ", Z:" + z + "\n";
	OutputDebugStringA(out.c_str());
}