#ifndef VULKAN_PHYSICAL_DEVICE_H_
#define VULKAN_PHYSICAL_DEVICE_H_

#include <vulkan/vulkan.h>

struct VulkanPhysicalDevice {
	VkPhysicalDevice device{ nullptr };

	VkPhysicalDeviceProperties properties;

	VkPhysicalDeviceFeatures features;

	VkPhysicalDeviceMemoryProperties memoryProperties;

	std::vector<VkQueueFamilyProperties> queueFamilyProperties;

	std::vector<std::string> supportedExtensions;

	operator VkPhysicalDevice() const
	{
		return device;
	}
};

#endif //VULKAN_PHYSICAL_DEVICE_H_
