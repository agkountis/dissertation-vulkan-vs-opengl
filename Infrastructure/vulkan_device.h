#ifndef VULKAN_DEVICE_H_
#define VULKAN_DEVICE_H_
#include <vulkan/vulkan.h>
#include "vulkan_physical_device.h"

class VulkanDevice {
private:
	VulkanPhysicalDevice m_PhysicalDevice;

	VkDevice m_LogicalDevice{ nullptr };

	bool PickPhysicalDevice(VkInstance instance);

public:
	~VulkanDevice();

	bool Initialize(VkInstance instance) noexcept;

	bool CreateLogicalDevice(VkPhysicalDeviceFeatures featuresToEnable,
	                         std::vector<const char*> extensionsToEnable,
	                         bool useSwapChain = true,
	                         VkQueueFlags requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);

	const VulkanPhysicalDevice& GetPhysicalDevice() const noexcept;
};

#endif //VULKAN_DEVICE_H_
