#include "gl_window.h"
#include "gl_application.h"
#include "logger.h"

// Static methods ------------------------
void GLWindow::OnWindowResize(GLFWwindow* window, i32 width, i32 height) noexcept
{
	auto app = static_cast<GLApplication*>(glfwGetWindowUserPointer(window));

	app->GetWindow().SetSize(Vec2ui{ width, height });
	app->Reshape(Vec2i{ width, height });
}
//----------------------------------------

GLWindow::~GLWindow()
{
	LOG("Cleaning up VulkanWindow (terminating glfw)");
	glfwTerminate();
}

bool GLWindow::Create(const std::string& title, const Vec2ui& size, const Vec2i& position,
	Application* application) noexcept
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

	m_Handle = glfwCreateWindow(size.x, size.y, title.c_str(), nullptr, nullptr);

	if (!m_Handle) {
		ERROR_LOG("Failed to create window.");
		return false;
	}

	glfwMakeContextCurrent(m_Handle);

	glfwSetWindowPos(m_Handle, position.x, position.y);

	glfwShowWindow(m_Handle);

	glfwSetWindowUserPointer(m_Handle, application);

	glfwSetWindowSizeCallback(m_Handle, OnWindowResize);

	SetSize(size);

	SetPosition(position);

	return true;
}

GLWindow::operator GLFWwindow*() const
{
	return m_Handle;
}
