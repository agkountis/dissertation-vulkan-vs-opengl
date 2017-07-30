#include "vulkan_window.h"
#include "logger.h"
#include "vulkan_application.h"

// Static methods ------------------------
void VulkanWindow::OnWindowResize(GLFWwindow* window, i32 width, i32 height) noexcept
{
	VulkanApplication* app{ static_cast<VulkanApplication*>(glfwGetWindowUserPointer(window)) };

	app->GetWindow().SetSize(Vec2ui{ width, height });

	app->Reshape(Vec2i{ width, height });
}
//----------------------------------------

VulkanWindow::~VulkanWindow()
{
	glfwTerminate();
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

bool VulkanWindow::Create(const std::string& title,
                          const Vec2ui& size,
                          const Vec2i& position,
                          Application* application) noexcept
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

	m_Handle = glfwCreateWindow(size.x, size.y, title.c_str(), nullptr, nullptr);

	if (!m_Handle) {
		ERROR_LOG("Failed to create window.");
		return false;
	}

	glfwSetWindowPos(m_Handle, position.x, position.y);

	glfwShowWindow(m_Handle);

	glfwSetWindowUserPointer(m_Handle, application);

	glfwSetWindowSizeCallback(m_Handle, OnWindowResize);

	SetSize(size);

	SetPosition(position);

	return true;
}

VulkanWindow::operator GLFWwindow*() const
{
	return m_Handle;
}
