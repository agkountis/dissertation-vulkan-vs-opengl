#include "vulkan_physical_device.h"

VulkanPhysicalDevice::operator VkPhysicalDevice() const noexcept
{
	return device;
}

ui32 VulkanPhysicalDevice::GetQueueFamilyIndex(VkQueueFlagBits queueFlagBits) noexcept
{
	if (queueFlagBits & VK_QUEUE_COMPUTE_BIT) {
		for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++) {
			if ((queueFamilyProperties[i].queueFlags & queueFlagBits) &&
			    ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)) {
				return i;
			}
		}
	}

	// Dedicated queue for transfer
	// Try to find a queue family index that supports transfer but not graphics and compute
	if (queueFlagBits & VK_QUEUE_TRANSFER_BIT) {
		for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++) {
			if ((queueFamilyProperties[i].queueFlags & queueFlagBits) &&
			    ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) &&
			    ((queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0)) {
				return i;
			}
		}
	}

	// For other queue types or if no separate compute queue is present, return the first one to support the requested flags
	for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++) {
		if (queueFamilyProperties[i].queueFlags & queueFlagBits) {
			return i;
		}
	}

	return std::numeric_limits<ui32>::max();
}

