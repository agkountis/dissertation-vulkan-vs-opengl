#include "application.h"


void Application::SetTermination(bool state) noexcept
{
	m_Terminate = state;
}

bool Application::ShouldTerminate() const noexcept
{
	return m_Terminate;
}

const Timer& Application::GetTimer() const noexcept
{
	return m_Timer;
}

bool Application::Initialize()
{
	//TODO

	return true;
}
