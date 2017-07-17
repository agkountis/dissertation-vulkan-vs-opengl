#ifndef VULKAN_DEPTH_STENCIL_H_
#define VULKAN_DEPTH_STENCIL_H_

#include <vulkan/vulkan.h>
#include "types.h"

class VulkanDevice;

class VulkanDepthStencil {
private:
	VkDevice m_pLogicalDevice{ nullptr };

	VkImage m_Image{ VK_NULL_HANDLE };

	VkDeviceMemory m_Memory{ VK_NULL_HANDLE };

	VkImageView m_ImageView{ VK_NULL_HANDLE };

public:
	~VulkanDepthStencil();

	VkImageView GetImageView() const noexcept;

	VkImage GetImage() const noexcept;

	bool Create(const VulkanDevice& logicalDevice, const Vec2ui& size, VkFormat format) noexcept;
};

#endif //VULKAN_DEPTH_STENCIL_H_
