#ifndef VULKAN_WINDOW_H_
#define VULKAN_WINDOW_H_

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "window.h"
#include <vector>

class Application;
class VulkanInstance;

class VulkanWindow : public Window {
private:
	VkInstance m_pInstance{ nullptr };

	GLFWwindow* m_Handle{ nullptr };

	static void OnWindowResize(GLFWwindow* window, i32 width, i32 height) noexcept;

public:
	~VulkanWindow();

	static std::vector<const char*> GetExtensions() noexcept;

	bool Create(const std::string title,
	            const Vec2ui& size,
	            const Vec2i& position,
	            Application* application) noexcept override;

	operator GLFWwindow*() const;
};


#endif //VULKAN_WINDOW_H_
