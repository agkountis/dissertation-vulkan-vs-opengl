#ifndef VULKAN_SEMAPHORE_H_
#define VULKAN_SEMAPHORE_H_

#include <vulkan/vulkan.h>

class VulkanSemaphore {
private:
	VkSemaphore m_Semaphore{ VK_NULL_HANDLE };

public:
	~VulkanSemaphore();

	bool Create() noexcept;

	void Destroy() const noexcept;

	operator VkSemaphore() const noexcept;

	const VkSemaphore* Get() const noexcept;

};

#endif //VULKAN_SEMAPHORE_H_
