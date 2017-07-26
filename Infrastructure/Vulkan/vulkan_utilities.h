#ifndef VULKAN_UTILITIES_H_
#define VULKAN_UTILITIES_H_

#include "types.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <string>

namespace VulkanUtilities
{
	struct QueueFamilyIndices {
		i32 GraphicsFamily{ -1 };

		i32 PresentFamily{ -1 };

		bool IsComplete() const noexcept
		{
			return GraphicsFamily >= 0 && PresentFamily >= 0;
		}
	};

	struct SwapChainSupportInfo {
		VkSurfaceCapabilitiesKHR Capabilities;
		std::vector<VkSurfaceFormatKHR> SurfaceFormats;
		std::vector<VkPresentModeKHR> PresentModes;

		bool IsAdequate() const noexcept
		{
			return !SurfaceFormats.empty() && !PresentModes.empty();
		}
	};

	QueueFamilyIndices QueryQueueFamilyIndices(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) noexcept;

	SwapChainSupportInfo QuerySwapChainSupportInfo(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) noexcept;

	bool CheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice, const std::vector<std::string>& requiredExtensions) noexcept;

	bool DeviceIsSuitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, const std::vector<std::string>& requiredExtensions) noexcept;
}

#endif // VULKAN_UTILITIES_H_
