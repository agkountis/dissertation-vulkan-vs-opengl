#include "demo_application.h"

//---------------------------------------------------------------------------------------------

DemoApplication::DemoApplication(const ApplicationSettings& settings)
	: GLApplication{ settings }
{
}

bool DemoApplication::Initialize() noexcept
{
	if (!GLApplication::Initialize()) {
		return false;
	}

	if (!m_DemoScene.Initialize()) {
		return false;
	}


	return true;
}

void DemoApplication::Update() noexcept
{
	m_DemoScene.Update(GetTimer().GetMsec(),
	                   GetTimer().GetDelta());
}

void DemoApplication::Draw() noexcept
{
	m_DemoScene.Draw();
}

void DemoApplication::OnResize(const Vec2i& size) noexcept
{
	LOG("RESIZE EVENT!");
}
