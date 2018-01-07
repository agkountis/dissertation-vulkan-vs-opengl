#include "demo_application.h"

DemoApplication::DemoApplication(const ApplicationSettings& settings) noexcept
	: GLApplication{ settings }
{
}

bool DemoApplication::Initialize() noexcept
{
	if (!GLApplication::Initialize()) {
		return false;
	}

	return m_DemoScene.Initialize();
}

void DemoApplication::Update() noexcept
{
	m_DemoScene.Update(GetTimer().GetMsec(), GetTimer().GetDelta());
}

void DemoApplication::Draw() noexcept
{
	m_DemoScene.Draw();
}

void DemoApplication::OnResize(const Vec2i& size) noexcept
{
}
