#include "vulkan_window.h"
#include "logger.h"
#include "vulkan_instance.h"
#include "vulkan_application.h"

// Static methods ------------------------
void VulkanWindow::OnWindowResize(GLFWwindow* window, i32 width, i32 height) noexcept
{
	VulkanApplication* app{ static_cast<VulkanApplication*>(glfwGetWindowUserPointer(window)) };

	app->GetWindow()->size = Vec2i{ width, height };

	//TODO: recreate the swapchain and resources that depend on it.
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

	handle = glfwCreateWindow(size.x, size.y, title.c_str(), nullptr, nullptr);

	glfwSetWindowUserPointer(handle, application);
	//TODO
	//glfwSetWindowSizeCallback(m_Window, )
}

VulkanWindow::~VulkanWindow()
{
	vkDestroySurfaceKHR(instance, surface, nullptr);

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

bool VulkanWindow::CreateSurface(const std::unique_ptr<VulkanInstance>& instance) noexcept
{
	VkResult result{ glfwCreateWindowSurface(*instance, handle, nullptr, &surface) };

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create window surface.");
		return false;
	}

	LOG("Window surface sucessfully created.");

	return true;
}


VulkanWindow::operator GLFWwindow*() const
{
	return handle;
}
