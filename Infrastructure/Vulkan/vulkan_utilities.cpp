#include "vulkan_utilities.h"
#include <string>
#include <vector>
#include <set>
#include "../logger.h"

namespace VulkanUtilities
{
	QueueFamilyIndices QueryQueueFamilyIndices(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) noexcept
	{
		QueueFamilyIndices queueFamilyIndices;

		ui32 queueFamilyCount{ 0 };

		// Get the queue family properties count
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilyProperties{ queueFamilyCount };

		// Get the queue family properties.
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

		// For every queue family property...
		for (ui32 i = 0; i < queueFamilyCount; ++i) {
			if (queueFamilyProperties[i].queueCount > 0) {

				// If it's a graphics queue...
				if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
					queueFamilyIndices.GraphicsFamily = i;
				}

				VkBool32 presentSupport{ false };
				vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);

				// If the queue supports presentation...
				if (presentSupport) {
					queueFamilyIndices.PresentFamily = i;
				}
			}

			// If both queue family indices have been selected stop iterating.
			if (queueFamilyIndices.IsComplete()) {
				break;
			}
		}

		return queueFamilyIndices;
	}

	SwapChainSupportInfo QuerySwapChainSupportInfo(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) noexcept
	{
		SwapChainSupportInfo info;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &info.Capabilities);

		ui32 formatCount{ 0 };
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);

		if (formatCount) {
			info.SurfaceFormats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, info.SurfaceFormats.data());
		}

		ui32 presentModeCount{ 0 };
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

		if (presentModeCount) {
			info.PresentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, info.PresentModes.data());
		}

		return info;
	}

	bool CheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice, const std::vector<std::string>& requiredExtensions) noexcept
	{
		ui32 extensionCount;
		
		// Get the extension count.
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions{ extensionCount };

		// Get the available extensions.
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

		// Copy the required extensions into a set to avoid duplicates.
		std::set<std::string> uniqueExtensions{ requiredExtensions.begin(), requiredExtensions.end() };

		// Remove every extension match.
		for (const auto& extension : availableExtensions) {
			uniqueExtensions.erase(extension.extensionName);
		}

		// Some of the extensions are not supported. Abort.
		if (!uniqueExtensions.empty()) {
			ERROR_LOG("Not all required extensions are supported:");
			
			for (const auto& extension : uniqueExtensions) {
				ERROR_LOG("\t >" + extension);
			}

			return false;
		}

		return true;
	}

	bool DeviceIsSuitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, const std::vector<std::string>& requiredExtensions) noexcept
	{
		QueueFamilyIndices indices{ QueryQueueFamilyIndices(physicalDevice, surface) };

		bool extensionsSupported{ CheckDeviceExtensionSupport(physicalDevice, requiredExtensions) };

		SwapChainSupportInfo swapChainSupport;
		if (extensionsSupported) {
			swapChainSupport = QuerySwapChainSupportInfo(physicalDevice, surface);
		}

		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);

		return indices.IsComplete() && extensionsSupported && swapChainSupport.IsAdequate() && supportedFeatures.samplerAnisotropy;
	}
}
