#include <string>
#include "application.h"

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

const Timer& Application::GetTimer() const noexcept
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

void Application::DestroyResource(const std::string& name) noexcept
{
	m_ResourceManager.DestroyResource(name);
}

void Application::DestroyResource(Resource* resource) noexcept
{
	m_ResourceManager.DestroyResource(resource);
}
