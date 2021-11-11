#pragma once
#include <functional>

class Timer
{
public:
	Timer();
	Timer(float length);
	Timer(float length, std::function<void(void)> func);
	~Timer();
	void Update(float deltaTime);

	void Start();
	void Stop();

	void SetLength(float length) { m_length = length; }
	void SetFunc(std::function<void(void)> func) { m_func = func; }
	bool GetActive() { return m_active; }
private:
	bool m_active = false;
	float m_length = 1.0f;
	float m_currentTime = 0.0f;
	std::function<void(void)> m_func = [&]() {};
};