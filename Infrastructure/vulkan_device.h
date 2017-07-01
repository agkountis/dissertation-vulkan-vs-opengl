#ifndef VULKAN_DEVICE_H_
#define VULKAN_DEVICE_H_
#include <vulkan/vulkan.h>

class VulkanDevice {
private:
	VkPhysicalDevice m_PhysicalDevice{ nullptr };

	VkDevice m_LogicalDevice{ nullptr };

	void PickPhysicalDevice(VkInstance instance);

public:
	explicit VulkanDevice(VkInstance instance);

	~VulkanDevice();
};

#endif //VULKAN_DEVICE_H_
