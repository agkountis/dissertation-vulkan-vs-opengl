#ifndef VULKAN_WINDOW_H_
#define VULKAN_WINDOW_H_
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "window.h"
#include <vector>
#include <memory>

class Application;
class VulkanInstance;

class VulkanWindow : public Window {
private:
	GLFWwindow* m_Window{ nullptr };

	VkSurfaceKHR m_Surface{ VK_NULL_HANDLE };

	VkSwapchainKHR m_Swapchain{ VK_NULL_HANDLE };

	static void OnWindowResize(GLFWwindow* window, i32 width, i32 height) noexcept;

public:
	VulkanWindow(const std::string& title, const Vec2i& size, const Vec2i& position, Application* application);

	static std::vector<const char*> GetExtensions() noexcept;

	bool CreateSurface(const std::unique_ptr<VulkanInstance>& instance) noexcept;

	i32 MainLoop() noexcept override;
};


#endif //VULKAN_WINDOW_H_
