#ifndef VULKAN_SWAPCHAIN_H_
#define VULKAN_SWAPCHAIN_H_

#include <vector>
#include <memory>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "types.h"

struct VulkanWindow;

struct SwapChainBuffer {
	VkImage image;
	VkImageView imageView;
};

class VulkanSwapChain {
private:
	VkInstance m_Instance{ nullptr };

	VkDevice m_LogicalDevice{ nullptr };

	VkPhysicalDevice m_PhysicalDevice{ nullptr };

	VkSurfaceKHR m_Surface{ VK_NULL_HANDLE };

	VkFormat m_Format{ VK_FORMAT_UNDEFINED };

	VkColorSpaceKHR m_ColorSpace{ VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

	VkSwapchainKHR m_SwapChain{ VK_NULL_HANDLE };

	ui32 m_ImageCount{ 0 };

	std::vector<VkImage> m_Images;

	std::vector<SwapChainBuffer> m_Buffers;

	ui32 m_PresentQueueIndex{ std::numeric_limits<ui32>::max() };

	bool InitializeSurface(const std::unique_ptr<VulkanWindow>& window) noexcept;

public:

	bool Initialize(VkInstance instance,
	                VkPhysicalDevice physicalDevice,
	                VkDevice logicalDevice,
	                const std::unique_ptr<VulkanWindow>& window) noexcept;

	bool Create(const Vec2i& size, bool vsync) noexcept;
};

#endif //VULKAN_SWAPCHAIN_H_
