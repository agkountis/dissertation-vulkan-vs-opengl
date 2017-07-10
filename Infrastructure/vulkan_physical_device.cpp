#include "vulkan_physical_device.h"

static std::vector<VkFormat> s_DepthFormats{
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM
};

VulkanPhysicalDevice::operator VkPhysicalDevice() const noexcept
{
	return device;
}

ui32 VulkanPhysicalDevice::GetQueueFamilyIndex(VkQueueFlagBits queueFlagBits) noexcept
{
	if (queueFlagBits & VK_QUEUE_COMPUTE_BIT) {
		for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++) {
			if (queueFamilyProperties[i].queueFlags & queueFlagBits &&
			    (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) {
				return i;
			}
		}
	}

	// Dedicated queue for transfer
	// Try to find a queue family index that supports transfer but not graphics and compute
	if (queueFlagBits & VK_QUEUE_TRANSFER_BIT) {
		for (ui32 i = 0; i < static_cast<ui32>(queueFamilyProperties.size()); ++i) {
			if (queueFamilyProperties[i].queueFlags & queueFlagBits &&
			    (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0 &&
			    (queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0) {
				return i;
			}
		}
	}

	// For other queue types or if no separate compute queue is present, return the first one to support the requested flags
	for (ui32 i = 0; i < static_cast<ui32>(queueFamilyProperties.size()); ++i) {
		if (queueFamilyProperties[i].queueFlags & queueFlagBits) {
			return i;
		}
	}

	return std::numeric_limits<ui32>::max();
}

VkFormat VulkanPhysicalDevice::GetSupportedDepthFormat() const noexcept
{
	for (const auto& depthFormat : s_DepthFormats) {

		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(device, depthFormat, &formatProperties);

		if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
			return depthFormat;
		}
	}

	return VK_FORMAT_UNDEFINED;
}

