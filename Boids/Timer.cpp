#include "Timer.h"

Timer::Timer()
{
}

Timer::Timer(float length)
{
	m_length = length;
}

Timer::Timer(float length, std::function<void(void)> func)
{
	m_length = length;
	m_func = func;
}

Timer::~Timer()
{
}

void Timer::Update(float deltaTime)
{
	m_currentTime += deltaTime;
	if (m_currentTime >= m_length)
	{
		m_func();
		m_active = false;
		m_currentTime = 0.0f;
	}
}

void Timer::Start()
{
	m_currentTime = 0.0f;
	m_active = true;
}

void Timer::Stop()
{
	m_currentTime = 0.0f;
	m_active = false;
}
