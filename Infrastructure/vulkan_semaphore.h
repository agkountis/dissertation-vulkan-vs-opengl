#ifndef VULKAN_SEMAPHORE_H_
#define VULKAN_SEMAPHORE_H_

#include <vulkan/vulkan.h>

class VulkanSemaphore {
private:
	VkDevice m_pLogicalDevice{ nullptr };

	VkSemaphore m_Semaphore{ VK_NULL_HANDLE };

public:
	~VulkanSemaphore();

	bool Create(VkDevice logicalDevice) noexcept;

	operator VkSemaphore() const noexcept;

	const VkSemaphore* operator&() const noexcept;
};

#endif //VULKAN_SEMAPHORE_H_
