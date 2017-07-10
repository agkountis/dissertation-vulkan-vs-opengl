#ifndef VULKAN_DEPTH_STENCIL_H_
#define VULKAN_DEPTH_STENCIL_H_

#include <vulkan/vulkan.h>

struct VulkanDepthStencil {
	VkDevice logicalDevice{ nullptr };

	VkImage image{ VK_NULL_HANDLE };

	VkDeviceMemory memory{ VK_NULL_HANDLE };

	VkImageView imageView{ VK_NULL_HANDLE };

	~VulkanDepthStencil();
};

#endif //VULKAN_DEPTH_STENCIL_H_
