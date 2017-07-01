#include "vulkan_window.h"

// Static methods ------------------------
void VulkanWindow::OnWindowResize(GLFWwindow* window, i32 width, i32 height) noexcept
{
}

//----------------------------------------

VulkanWindow::VulkanWindow(const std::string& title,
                           const Vec2i& size,
                           const Vec2i& position,
                           Application* application)
	: Window{ title, size, position }
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	m_Window = glfwCreateWindow(size.x, size.y, title.c_str(), nullptr, nullptr);

	glfwSetWindowUserPointer(m_Window, application);

	//TODO
	//glfwSetWindowSizeCallback(m_Window, )
}

std::vector<const char*> VulkanWindow::GetExtensions() noexcept
{
	std::vector<const char*> extensions;

	ui32 glfwExtensionCount{ 0 };
	auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	for (ui32 i = 0; i < glfwExtensionCount; ++i) {
		extensions.push_back(glfwExtensions[i]);
	}

	return std::move(extensions);
}

i32 VulkanWindow::MainLoop() noexcept
{
	while (!glfwWindowShouldClose(m_Window)) {
		glfwPollEvents();
	}

	return EXIT_SUCCESS;
}
