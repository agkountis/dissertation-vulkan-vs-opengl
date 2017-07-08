#ifndef VULKAN_PHYSICAL_DEVICE_H_
#define VULKAN_PHYSICAL_DEVICE_H_

#include <vulkan/vulkan.h>
#include <vector>
#include "types.h"

struct VulkanPhysicalDevice {
	VkPhysicalDevice device{ nullptr };

	VkPhysicalDeviceProperties properties;

	VkPhysicalDeviceFeatures features;

	VkPhysicalDeviceMemoryProperties memoryProperties;

	std::vector<VkQueueFamilyProperties> queueFamilyProperties;

	std::vector<std::string> supportedExtensions;

	operator VkPhysicalDevice() const noexcept;

	ui32 GetQueueFamilyIndex(VkQueueFlagBits queueFlagBits) noexcept;
};

#endif //VULKAN_PHYSICAL_DEVICE_H_
