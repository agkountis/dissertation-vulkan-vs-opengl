#ifndef VULKAN_COMMAND_POOL_H_
#define VULKAN_COMMAND_POOL_H_

#include <vulkan/vulkan.h>
#include "../types.h"

class VulkanCommandPool {
private:
	VkDevice m_pLogicalDevice{ nullptr };

	VkCommandPool m_CommandPool{ VK_NULL_HANDLE };

public:
	~VulkanCommandPool();

	bool Create(VkDevice logicalDevice, ui32 queueFamilyIndex, VkCommandPoolCreateFlags createFlags) noexcept;

	operator VkCommandPool() const noexcept;

	VkCommandPool* operator&() noexcept;

	const VkCommandPool* operator&() const noexcept;
};

#endif //VULKAN_COMMAND_POOL_H_