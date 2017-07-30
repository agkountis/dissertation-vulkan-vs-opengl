#ifndef VULKAN_DEPTH_STENCIL_H_
#define VULKAN_DEPTH_STENCIL_H_

#include <vulkan/vulkan.h>
#include "types.h"

class VulkanDevice;

class VulkanDepthStencil {
private:
	VkImage m_Image{ VK_NULL_HANDLE };

	VkDeviceMemory m_Memory{ VK_NULL_HANDLE };

	VkImageView m_ImageView{ VK_NULL_HANDLE };

	VkFormat m_Format{ VK_FORMAT_UNDEFINED };

public:
	~VulkanDepthStencil();

	VkImageView GetImageView() const noexcept;

	VkImage GetImage() const noexcept;

	VkFormat GetFormat() const noexcept;

	bool Create(const Vec2ui& size, VkFormat format) noexcept;

	void Destroy() const noexcept;
};

#endif //VULKAN_DEPTH_STENCIL_H_
