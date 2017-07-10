#include "vulkan_depth_stencil.h"


VulkanDepthStencil::~VulkanDepthStencil()
{
	vkDestroyImage(logicalDevice, image, nullptr);

	vkFreeMemory(logicalDevice, memory, nullptr);

	vkDestroyImageView(logicalDevice, imageView, nullptr);
}
