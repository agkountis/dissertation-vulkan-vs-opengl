#include "GL/glew.h"
#include "gl_application.h"
#include "logger.h"

GLApplication::GLApplication(const ApplicationSettings& settings) noexcept
	: Application{ settings }
{
}

bool GLApplication::Initialize() noexcept
{
	if (!Application::Initialize()) {
		return false;
	}

	//todo : Register app to the GLAPP CONTEXT file;

	const auto& settings = GetSettings();

	if (!m_Window.Create(settings.name,
	                     settings.windowResolution,
	                     settings.windowPosition,
	                     this)) {
		return false;
	}

	if (settings.vsync) {
		glfwSwapInterval(1);
	}

	glewExperimental = true;
	const auto error = glewInit();

	if (error != GLEW_OK) {
		ERROR_LOG(glewGetErrorString(error));
		return false;
	}

	if (!GLEW_ARB_gl_spirv) {
		ERROR_LOG("GL_ARB_gl_spirv extension not supported. Aborting!");
		return false;
	}

	if (!GLEW_ARB_separate_shader_objects) {
		ERROR_LOG("GL_ARB_separate_shader_objects extension not supported. Aborting!");
		return false;
	}

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	glDisable(GL_MULTISAMPLE);

	return true;
}

i32 GLApplication::Run() noexcept
{
	GetTimer().Start();
	while (!glfwWindowShouldClose(m_Window) && !ShouldTerminate()) {
		glfwPollEvents();

		Update();
		PreDraw();
		Draw();
		PostDraw();

		glfwSwapBuffers(m_Window);
	}

	return 0;
}

void GLApplication::PreDraw() noexcept
{
}

void GLApplication::PostDraw() noexcept
{
}

GLWindow& GLApplication::GetWindow() noexcept
{
	return m_Window;
}

void GLApplication::Reshape(const Vec2ui& size) noexcept
{
}
