#ifndef VULKAN_WINDOW_H_
#define VULKAN_WINDOW_H_
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include "window.h"

class VulkanWindow : public Window {
private:
	GLFWwindow* m_Window{ nullptr };

	static void OnWindowResize(GLFWwindow* window, i32 width, i32 height) noexcept;

public:
	VulkanWindow(const std::string& title, const Vec2i& size, const Vec2i& position);
};


#endif //VULKAN_WINDOW_H_
