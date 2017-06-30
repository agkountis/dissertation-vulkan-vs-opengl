#include "vulkan_window.h"

// Static methods ------------------------
void VulkanWindow::OnWindowResize(GLFWwindow* window, i32 width, i32 height) noexcept
{
		
}
//----------------------------------------

VulkanWindow::VulkanWindow(const std::string& title,
                           const Vec2i& size,
                           const Vec2i& position)
	: Window{ title, size, position }
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	m_Window = glfwCreateWindow(size.x, size.y, title.c_str(), nullptr, nullptr);

	glfwSetWindowUserPointer(m_Window, this);

	//TODO
	//glfwSetWindowSizeCallback(m_Window, )
}
