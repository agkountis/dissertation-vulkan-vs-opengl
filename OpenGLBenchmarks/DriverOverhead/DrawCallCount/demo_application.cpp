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

	return true;
}

void DemoApplication::Update() noexcept
{

}

void DemoApplication::Draw() noexcept
{
	glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


}

void DemoApplication:: OnResize(const Vec2i& size) noexcept
{

}
