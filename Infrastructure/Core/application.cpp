#include <string>
#include "application.h"
#include "cfg.h"

Application::Application(const ApplicationSettings& settings)
		: m_Settings{ settings }
{
}

void Application::SetTermination(bool state) noexcept
{
	m_Terminate = state;
}

bool Application::ShouldTerminate() const noexcept
{
	return m_Terminate;
}

Timer& Application::GetTimer() noexcept
{
	return m_Timer;
}

const ApplicationSettings& Application::GetSettings() const noexcept
{
	return m_Settings;
}

void Application::SetSettings(const ApplicationSettings& settings) noexcept
{
	m_Settings = settings;
}

float Application::GetDuration() const noexcept
{
	return m_Duration;
}

bool Application::Initialize() noexcept
{
	ConfigFile cfg{ "config/config.cfg" };

	if (!cfg.IsOpen()) {
		ERROR_LOG("Failed to open configuration file");
		return false;
	}

	m_Duration = cfg.GetFloat("attributes.duration", -1.0f);

	return true;
}
