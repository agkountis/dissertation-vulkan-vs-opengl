#ifndef VULKAN_SWAPCHAIN_H_
#define VULKAN_SWAPCHAIN_H_

#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "types.h"

class VulkanWindow;

struct SwapChainBuffer {
	VkImage image;
	VkImageView imageView;
};

class VulkanSwapChain {
private:
	VkInstance m_Instance;

	VkDevice m_LogicalDevice;

	VkPhysicalDevice m_PhysicalDevice;

	VkSurfaceKHR m_Surface;

	VkFormat m_Format;

	VkColorSpaceKHR m_ColorSpace;

	VkSwapchainKHR m_SwapChain{ VK_NULL_HANDLE };

	ui32 m_ImageCount;

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
