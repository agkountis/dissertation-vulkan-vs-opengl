#ifndef VULKAN_DEVICE_H_
#define VULKAN_DEVICE_H_

#include <vulkan/vulkan.h>
#include "vulkan_physical_device.h"
#include "vulkan_buffer.h"

/**
 * \brief A structure that contains the device queue family indices.
 */
struct QueueFamilyIndices {
	ui32 graphics{ std::numeric_limits<ui32>::max() };
	ui32 compute{ std::numeric_limits<ui32>::max() };
	ui32 transfer{ std::numeric_limits<ui32>::max() };
};

/**
 * \brief Scoped enum that enumerates the device queue families.
 */
enum class QueueFamily {
	GRAPHICS,
	TRANSFER,
	COMPUTE,
	PRESENT
};

class VulkanDevice {
private:
	/**
	 * \brief The Vulkan physical device.
	 * \details Represents a physical device (GPU).
	 */
	VulkanPhysicalDevice m_PhysicalDevice;

	/**
	 * \brief The Vulkan logical device.
	 * \details Used throughout the application to interface with the physical device.
	 */
	VkDevice m_LogicalDevice{ nullptr };

	/**
	 * \brief The indices of the queue families enabled on the physical device.
	 */
	QueueFamilyIndices m_QueueFamilyIndices;

	/**
	 * \brief Command pool used to allocate command buffers
	 * for single use commands, like buffer copies or image layout transitions.
	 */
	VkCommandPool m_CommandPool{ VK_NULL_HANDLE };

	/**
	 * \brief The graphics queue.
	 */
	VkQueue m_GraphicsQueue{ nullptr };

	/**
	 * \brief The transfer queue.
	 */
	VkQueue m_TransferQueue{ nullptr };

	/**
	 * \brief The compute queue.
	 */
	VkQueue m_ComputeQueue{ nullptr };

	/**
	 * \brief Features enabled on this device.
	 */
	VkPhysicalDeviceFeatures m_EnabledFeatures;

	/**
	 * \brief Select the most suitable physical device (GPU).
	 * \param instance The Vulkan instance.
	 * \return TRUE if successful, FALSE otherwise.
	 */
	bool PickPhysicalDevice(VkInstance instance);

public:
	~VulkanDevice();

	/**
	 * \brief Initializes the device class.
	 * \details Selects the physical device.
	 * \param instance The active Vulkan instance.
	 * \return TRUE if successful, FALSE otherwise.
	 */
	bool Initialize(VkInstance instance) noexcept;

	/**
	 * \brief Creates the Vulkan logical device.
	 * \param featuresToEnable The requested features to be enabled.
	 * \param extensionsToEnable The requested extensions to be enabled.
	 * \param useSwapChain Flag that indicates if a swap chain needs
	 * to be used. (false for a non-rendering device)
	 * \param requestedQueueTypes The queue types to be enabled on the device.
	 * \return TRUE if successful, FALSE otherwise.
	 */
	bool CreateLogicalDevice(VkPhysicalDeviceFeatures featuresToEnable,
	                         std::vector<const char *> extensionsToEnable,
	                         bool useSwapChain = true,
	                         VkQueueFlags requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);

	/**
	 * \brief Returns the selected physical device.
	 * \return The selected physical device.
	 */
	const VulkanPhysicalDevice& GetPhysicalDevice() const noexcept;

	/**
	 * \brief Returns the index of the appropriate device memory type.
	 * \param memoryTypeMask The memory requirements.
	 * \param memoryPropertyFlags The memory properties.
	 * \return The appropriate memory type index.
	 */
	ui32 GetMemoryTypeIndex(ui32 memoryTypeMask, VkMemoryPropertyFlags memoryPropertyFlags) const noexcept;

	/**
	 * \brief Creates a Vulkan command pool.
	 * \param queueFamilyIndex The index of the queue family the buffers
	 * allocated from this pool are going to be submitted to.
	 * \param createFlags Vulkan command pool creation flags.
	 * \return The newly created command pool.
	 */
	VkCommandPool CreateCommandPool(ui32 queueFamilyIndex,
	                                VkCommandPoolCreateFlags createFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT) const noexcept;

	/**
	 * \brief Creates a Vulkan buffer.
	 * \param usageFlags Flags that specify how the buffer is going to be used.
	 * \param memoryPropertyFlags Flags that specify the memory properties of the buffer.
	 * \param buffer The VulkanBuffer to be created.
	 * \param size The size of the buffer.
	 * \param data (Optional) If provided, the data will be copied into the buffer.
	 * \return TRUE if successful, FALSE otherwise.
	 */
	bool CreateBuffer(VkBufferUsageFlags usageFlags,
	                  VkMemoryPropertyFlags memoryPropertyFlags,
	                  VulkanBuffer& buffer,
	                  VkDeviceSize size,
	                  void *data = nullptr) const noexcept;

	VkCommandBuffer CreateCommandBuffer(VkCommandBufferLevel commandBufferLevel) const noexcept;

	bool SubmitCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, VkFence fence = nullptr) const noexcept;

	/**
	 * \brief Copies the contents of the source buffer to the destination buffer.
	 * \param source The source buffer.
	 * \param destination The destination buffer.
	 * \param transferQueue The queue to be used for the data transfer.
	 * \param copyRegion (Optional) If provided this specific copy region is going to be used.
	 * If not provided the whole buffer will be copied.
	 */
	bool CopyBuffer(const VulkanBuffer& source,
	                const VulkanBuffer& destination,
	                VkQueue transferQueue,
	                VkBufferCopy* copyRegion = nullptr) const noexcept;

	/**
	 * \brief Returns the requested QueueFamily.
	 * \param queueFamily The requested QueueFamily.
	 * \return The requested queue.
	 */
	VkQueue GetQueue(QueueFamily queueFamily) const noexcept;

	/**
	 * \brief Automatic conversion to Vk device.
	 * \details Utility that allows this class to be used directly in Vulkan functions.
	 * \return The VkDevice handle the class contains.
	 */
	operator VkDevice() const noexcept;
};

#endif //VULKAN_DEVICE_H_
