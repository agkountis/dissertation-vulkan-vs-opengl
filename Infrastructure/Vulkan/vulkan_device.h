#ifndef VULKAN_DEVICE_H_
#define VULKAN_DEVICE_H_

#include <vulkan/vulkan.h>
#include "vulkan_physical_device.h"
#include "vulkan_buffer.h"

struct QueueFamilyIndices {
	ui32 graphics{ std::numeric_limits<ui32>::max() };
	ui32 compute{ std::numeric_limits<ui32>::max() };
	ui32 transfer{ std::numeric_limits<ui32>::max() };
};

enum class QueueFamily {
	GRAPHICS,
	TRANSFER,
	COMPUTE,
	PRESENT
};

class VulkanDevice {
private:
	VulkanPhysicalDevice m_PhysicalDevice;

	VkDevice m_LogicalDevice{ nullptr };

	QueueFamilyIndices m_QueueFamilyIndices;

	VkCommandPool m_CommandPool{ VK_NULL_HANDLE };

	VkQueue m_GraphicsQueue{ nullptr };

	VkPhysicalDeviceFeatures m_EnabledFeatures;

	bool PickPhysicalDevice(VkInstance instance);

public:
	~VulkanDevice();

	bool Initialize(VkInstance instance) noexcept;

	bool CreateLogicalDevice(VkPhysicalDeviceFeatures featuresToEnable,
	                         std::vector<const char *> extensionsToEnable,
	                         bool useSwapChain = true,
	                         VkQueueFlags requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);

	const VulkanPhysicalDevice& GetPhysicalDevice() const noexcept;

	ui32 GetMemoryTypeIndex(ui32 memoryTypeMask, VkMemoryPropertyFlags memoryPropertyFlags) const noexcept;

	VkCommandPool CreateCommandPool(ui32 queueFamilyIndex,
	                                VkCommandPoolCreateFlags createFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT) const noexcept;

	bool CreateBuffer(VkBufferUsageFlags usageFlags,
	                  VkMemoryPropertyFlags memoryPropertyFlags,
	                  VulkanBuffer& buffer,
	                  VkDeviceSize size,
	                  void *data = nullptr) const noexcept;

	VkQueue GetQueue(QueueFamily queueFamily) const noexcept;

	operator VkDevice() const noexcept;
};

#endif //VULKAN_DEVICE_H_
