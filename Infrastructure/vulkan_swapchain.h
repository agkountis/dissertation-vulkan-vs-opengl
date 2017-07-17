#ifndef VULKAN_SWAPCHAIN_H_
#define VULKAN_SWAPCHAIN_H_

#include <vector>
#include <memory>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "types.h"

class VulkanWindow;

class VulkanSwapChain {
private:
	VkInstance m_Instance{ nullptr };

	VkDevice m_LogicalDevice{ nullptr };

	VkPhysicalDevice m_PhysicalDevice{ nullptr };

	VkSurfaceKHR m_Surface{ VK_NULL_HANDLE };

	VkFormat m_Format{ VK_FORMAT_UNDEFINED };

	VkColorSpaceKHR m_ColorSpace{ VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

	VkSwapchainKHR m_SwapChain{ VK_NULL_HANDLE };

	std::vector<VkImage> m_Images;

	std::vector<VkImageView> m_ImageViews;

	ui32 m_GraphicsAndPresentQueueIndex{ std::numeric_limits<ui32>::max() };

	VkExtent2D m_Extent;

	bool InitializeSurface(const VulkanWindow& window) noexcept;

public:
	~VulkanSwapChain();

	bool Initialize(VkInstance instance,
	                VkPhysicalDevice physicalDevice,
	                VkDevice logicalDevice,
	                const VulkanWindow& window) noexcept;

	bool Create(const Vec2i& size, bool vsync) noexcept;

	const std::vector<VkImage>& GetImages() const noexcept;

	const std::vector<VkImageView>& GetImageViews() const noexcept;

	ui32 GetQueueIndex() const noexcept;

	VkFormat GetFormat() const noexcept;

	const VkExtent2D& GetExtent() const noexcept;
};

#endif //VULKAN_SWAPCHAIN_H_
