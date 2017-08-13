#include <vector>
#include <cstring>
#include <limits>
#include "vulkan_device.h"
#include "logger.h"

// Private functions -------------------------------------------
bool VulkanDevice::PickPhysicalDevice(VkInstance instance)
{
	ui32 gpuCount{ 0 };

	// Get number of available physical devices
	VkResult result{ vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr) };

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to get physical device count.");
		return false;
	}

	if (!gpuCount) {
		ERROR_LOG("Fatal error! no GPUs detected.");
		return false;
	}

	// Enumerate devices
	std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
	result = vkEnumeratePhysicalDevices(instance, &gpuCount, physicalDevices.data());

	if (result != VK_SUCCESS) {
		ERROR_LOG("Could not get physical devices.");
		return false;
	}

	//Just pick the 1st one for now.
	m_PhysicalDevice.device = physicalDevices[0];

	vkGetPhysicalDeviceProperties(m_PhysicalDevice, &m_PhysicalDevice.properties);

	// Features should be checked by the examples before using them
	vkGetPhysicalDeviceFeatures(m_PhysicalDevice, &m_PhysicalDevice.features);

	// Memory properties are used regularly for creating all kinds of buffers
	vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &m_PhysicalDevice.memoryProperties);

	// Queue family properties, used for setting up requested queues upon device creation
	ui32 queueFamilyCount{ 0 };
	vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, nullptr);

	if (!queueFamilyCount) {
		ERROR_LOG("Fatal error! No available queue families.");
		return false;
	}

	m_PhysicalDevice.queueFamilyProperties.resize(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice,
	                                         &queueFamilyCount,
	                                         m_PhysicalDevice.queueFamilyProperties.data());

	// Get list of supported extensions
	ui32 extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &extensionCount, nullptr);
	if (extensionCount > 0) {

		std::vector<VkExtensionProperties> extensions{ extensionCount };

		result = vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &extensionCount, &extensions.front());

		if (result != VK_SUCCESS) {
			ERROR_LOG("Failed to get physical device extension properties.");
			return false;
		}

		for (const auto& extension : extensions) {
			m_PhysicalDevice.supportedExtensions.emplace_back(extension.extensionName);
		}
	}

	return true;
}
// -------------------------------------------------------------

VulkanDevice::~VulkanDevice()
{
	LOG("Cleaning up VulkanDevice");
	vkDestroyCommandPool(m_LogicalDevice, m_CommandPool, nullptr);
	vkDestroyDevice(m_LogicalDevice, nullptr);
}

bool VulkanDevice::Initialize(VkInstance instance) noexcept
{
	if (!PickPhysicalDevice(instance)) {
		ERROR_LOG("Failed to pick physical device");
		return false;
	}

	return true;
}

bool VulkanDevice::CreateLogicalDevice(VkPhysicalDeviceFeatures featuresToEnable,
                                       std::vector<const char*> extensionsToEnable,
                                       bool useSwapChain,
                                       VkQueueFlags requestedQueueTypes)
{
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};

	const float defaultQueuePriority{ 0.0f };

	if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT) {
		m_QueueFamilyIndices.graphics = m_PhysicalDevice.GetQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);

		if (m_QueueFamilyIndices.graphics == std::numeric_limits<ui32>::max()) {
			ERROR_LOG("Could not find a matching queue family.");
			return false;
		}

		VkDeviceQueueCreateInfo queueInfo{};
		queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo.queueFamilyIndex = m_QueueFamilyIndices.graphics;
		queueInfo.queueCount = 1;
		queueInfo.pQueuePriorities = &defaultQueuePriority;
		queueCreateInfos.push_back(queueInfo);
	} else {
		m_QueueFamilyIndices.graphics = VK_NULL_HANDLE;
	}

	if (requestedQueueTypes & VK_QUEUE_COMPUTE_BIT) {
		m_QueueFamilyIndices.compute = m_PhysicalDevice.GetQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT);

		if (m_QueueFamilyIndices.compute == std::numeric_limits<ui32>::max()) {
			ERROR_LOG("Failed to find a matching queue family.");
			return false;
		}

		if (m_QueueFamilyIndices.compute != m_QueueFamilyIndices.graphics) {
			// If compute family index differs, we need an additional queue create info for the compute queue
			VkDeviceQueueCreateInfo queueInfo{};
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.queueFamilyIndex = m_QueueFamilyIndices.compute;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &defaultQueuePriority;
			queueCreateInfos.push_back(queueInfo);
		}
	} else {
		// Else we use the same queue
		m_QueueFamilyIndices.compute = m_QueueFamilyIndices.graphics;
	}

	// Dedicated transfer queue
	if (requestedQueueTypes & VK_QUEUE_TRANSFER_BIT) {
		m_QueueFamilyIndices.transfer = m_PhysicalDevice.GetQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT);

		if (m_QueueFamilyIndices.transfer == std::numeric_limits<ui32>::max()) {
			ERROR_LOG("Failed to find a matching queue family");
			return false;
		}

		if ((m_QueueFamilyIndices.transfer != m_QueueFamilyIndices.graphics) &&
		    (m_QueueFamilyIndices.transfer != m_QueueFamilyIndices.compute)) {
			// If compute family index differs, we need an additional queue create info for the compute queue
			VkDeviceQueueCreateInfo queueInfo{};
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.queueFamilyIndex = m_QueueFamilyIndices.transfer;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &defaultQueuePriority;
			queueCreateInfos.push_back(queueInfo);
		}
	} else {
		// Else we use the same queue
		m_QueueFamilyIndices.transfer = m_QueueFamilyIndices.graphics;
	}

	std::vector<const char*> deviceExtensions{ std::move(extensionsToEnable) };
	if (useSwapChain) {
		// If the device will be used for presenting to a display via a swapchain we need to request the swapchain extension
		deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	}

	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = static_cast<ui32>(queueCreateInfos.size());
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.pEnabledFeatures = &featuresToEnable;

	if (!deviceExtensions.empty()) {
		deviceCreateInfo.enabledExtensionCount = static_cast<ui32>(deviceExtensions.size());
		deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
	}

	VkResult result{ vkCreateDevice(m_PhysicalDevice, &deviceCreateInfo, nullptr, &m_LogicalDevice) };

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create logical device.");
		return false;
	}

	m_CommandPool = CreateCommandPool(m_QueueFamilyIndices.graphics);

	if (!m_CommandPool) {
		ERROR_LOG("Failed to create command pool");
		return false;
	}

	m_EnabledFeatures = featuresToEnable;

	vkGetDeviceQueue(m_LogicalDevice, m_QueueFamilyIndices.graphics, 0, &m_GraphicsQueue);
	vkGetDeviceQueue(m_LogicalDevice, m_QueueFamilyIndices.transfer, 0, &m_TransferQueue);
	vkGetDeviceQueue(m_LogicalDevice, m_QueueFamilyIndices.compute, 0, &m_ComputeQueue);

	return true;
}

const VulkanPhysicalDevice& VulkanDevice::GetPhysicalDevice() const noexcept
{
	return m_PhysicalDevice;
}

ui32 VulkanDevice::GetMemoryTypeIndex(ui32 memoryTypeMask, VkMemoryPropertyFlags memoryPropertyFlags) const noexcept
{
	for (uint32_t i = 0; i < m_PhysicalDevice.memoryProperties.memoryTypeCount; i++) {
		if ((memoryTypeMask & (1 << i)) &&
		    (m_PhysicalDevice.memoryProperties.memoryTypes[i].propertyFlags & memoryPropertyFlags) ==
		    memoryPropertyFlags) {
			return i;
		}
	}

	ERROR_LOG("Could not find suitable m_Memory type.");
	return std::numeric_limits<ui32>::max();
}

VkCommandPool
VulkanDevice::CreateCommandPool(ui32 queueFamilyIndex, VkCommandPoolCreateFlags createFlags) const noexcept
{
	VkCommandPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.queueFamilyIndex = queueFamilyIndex;
	createInfo.flags = createFlags;

	VkCommandPool commandPool{ VK_NULL_HANDLE };
	vkCreateCommandPool(m_LogicalDevice, &createInfo, nullptr, &commandPool);

	return commandPool;
}

bool VulkanDevice::CreateBuffer(VkBufferUsageFlags usageFlags,
                                VkMemoryPropertyFlags memoryPropertyFlags,
                                VulkanBuffer& buffer,
                                VkDeviceSize size,
                                void* data) const noexcept
{
	// Create the m_Buffer handle
	VkBufferCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.usage = usageFlags;
	createInfo.size = size;
	VkResult result{ vkCreateBuffer(m_LogicalDevice, &createInfo, nullptr, &buffer.buffer) };

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create m_Buffer.");
		return false;
	}

	VkMemoryRequirements memoryRequirements{};
	vkGetBufferMemoryRequirements(m_LogicalDevice, buffer.buffer, &memoryRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo{};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;

	ui32 memoryTypeIndex{ GetMemoryTypeIndex(memoryRequirements.memoryTypeBits, memoryPropertyFlags) };

	if (memoryTypeIndex == std::numeric_limits<ui32>::max()) {
		return false;
	}

	memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;

	result = vkAllocateMemory(m_LogicalDevice, &memoryAllocateInfo, nullptr, &buffer.memory);

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to allocate m_Buffer m_Memory");
		return false;
	}

	buffer.memoryAlignment = memoryRequirements.alignment;
	buffer.size = memoryAllocateInfo.allocationSize;
	buffer.usageFlags = usageFlags;
	buffer.memoryPropertyFlags = memoryPropertyFlags;

	if (data != nullptr) {
		result = buffer.Map();

		if (result != VK_SUCCESS) {
			ERROR_LOG("Failed to map buffer memory.");
			return false;
		}

		memcpy(buffer.data, data, size);
		buffer.Unmap();
	}

	buffer.InitializeDescriptor();

	result = buffer.Bind();

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to bind buffer memory");
		return false;
	}

	return true;
}

bool VulkanDevice::CreateImage(const Vec2ui& imageDimensions,
                               VkFormat format,
                               VkImageTiling imageTiling,
                               VkImageUsageFlags imageUsageFlags,
                               VkMemoryPropertyFlags memoryPropertyFlags,
                               VkImage& image,
                               VkDeviceMemory& imageMemory) const noexcept
{
	VkImageCreateInfo imageCreateInfo{};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.extent.width = imageDimensions.x;
	imageCreateInfo.extent.height = imageDimensions.y;
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.format = format;
	imageCreateInfo.tiling = imageTiling;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
	imageCreateInfo.usage = imageUsageFlags;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkResult result{ vkCreateImage(m_LogicalDevice, &imageCreateInfo, nullptr, &image) };

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create image.");
		return false;
	}

	VkMemoryRequirements memoryRequirements{};
	vkGetImageMemoryRequirements(m_LogicalDevice, image, &memoryRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo{};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;

	ui32 memoryTypeIndex{ GetMemoryTypeIndex(memoryRequirements.memoryTypeBits, memoryPropertyFlags) };

	if (memoryTypeIndex == std::numeric_limits<ui32>::max()) {
		return false;
	}

	memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;

	result = vkAllocateMemory(m_LogicalDevice, &memoryAllocateInfo, nullptr, &imageMemory);

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to allocate image memory.");
		return false;
	}

	vkBindImageMemory(m_LogicalDevice, image, imageMemory, 0);

	return true;
}

VkCommandBuffer VulkanDevice::CreateCommandBuffer(VkCommandBufferLevel commandBufferLevel) const noexcept
{
	VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandBufferCount = 1;
	commandBufferAllocateInfo.level = commandBufferLevel;
	commandBufferAllocateInfo.pNext = nullptr;
	commandBufferAllocateInfo.commandPool = m_CommandPool;

	VkCommandBuffer commandBuffer{ nullptr };

	VkResult result{ vkAllocateCommandBuffers(m_LogicalDevice, &commandBufferAllocateInfo, &commandBuffer) };

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to allocate command m_Buffer.");
	}

	return commandBuffer;
}

bool VulkanDevice::SubmitCommandBuffer(VkCommandBuffer commandBuffer,
                                       VkQueue queue,
                                       VkFence fence,
                                       bool freeCommandBuffer,
                                       bool destroyFence) const noexcept
{
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(queue, 1, &submitInfo, fence);

	if (fence) {
		VkResult result{ vkWaitForFences(m_LogicalDevice, 1, &fence, VK_TRUE, std::numeric_limits<ui64>::max()) };

		if (result != VK_SUCCESS) {
			ERROR_LOG("Wait for fences failed.");
			return false;
		}

		if (destroyFence) {
			vkDestroyFence(m_LogicalDevice, fence, nullptr);
		}
	}

	if (freeCommandBuffer) {
		vkFreeCommandBuffers(m_LogicalDevice, m_CommandPool, 1, &commandBuffer);
	}

	return true;
}

bool VulkanDevice::CopyBuffer(const VulkanBuffer& source,
                              const VulkanBuffer& destination,
                              VkQueue transferQueue,
                              VkBufferCopy* copyRegion) const noexcept
{
	assert(destination.size <= source.size);
	assert(source.buffer && destination.buffer);

	VkCommandBuffer commandBuffer{ CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY) };

	VkCommandBufferBeginInfo commandBufferBeginInfo{};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	VkResult result{ vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo) };

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to begin command buffer. Buffer copy failed.");
		return false;
	}

	VkBufferCopy bufferCopy{};
	if (!copyRegion) {
		bufferCopy.size = source.size;
	} else {
		bufferCopy = *copyRegion;
	}

	vkCmdCopyBuffer(commandBuffer, source.buffer, destination.buffer, 1, &bufferCopy);

	vkEndCommandBuffer(commandBuffer);

	VkFenceCreateInfo fenceCreateInfo{};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_NULL_HANDLE;

	VkFence fence{ VK_NULL_HANDLE };

	result = vkCreateFence(m_LogicalDevice, &fenceCreateInfo, nullptr, &fence);

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create fence.");
		return false;
	}

	if (!SubmitCommandBuffer(commandBuffer, transferQueue, fence)) {
		ERROR_LOG("Command buffer submission failed.");
		return false;
	}

	return true;
}

bool VulkanDevice::CopyBufferToImage(const VulkanBuffer& source,
                                     VkImage destination,
                                     const Vec2ui& imageDimensions) const noexcept
{

	VkCommandBuffer commandBuffer{ CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY) };

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = VkOffset3D{ 0, 0, 0 };
	region.imageExtent = VkExtent3D{ imageDimensions.x, imageDimensions.y, 1 };

	VkCommandBufferBeginInfo commandBufferBeginInfo{};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

	vkCmdCopyBufferToImage(
			commandBuffer,
			source.buffer,
			destination,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region);

	vkEndCommandBuffer(commandBuffer);

	VkFenceCreateInfo fenceCreateInfo{};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_NULL_HANDLE;

	VkFence fence{ VK_NULL_HANDLE };

	VkResult result{ vkCreateFence(m_LogicalDevice, &fenceCreateInfo, nullptr, &fence) };

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create fence.");
		return false;
	}

	if (!SubmitCommandBuffer(commandBuffer, m_TransferQueue, fence)) {
		ERROR_LOG("Command buffer submission failed.");
		return false;
	}

	return true;
}

bool VulkanDevice::TransitionImageLayout(VkImage image,
                                         VkFormat imageFormat,
                                         VkImageLayout oldLayout,
                                         VkImageLayout newLayout,
                                         VkPipelineStageFlags sourceStageFlags,
                                         VkPipelineStageFlags destinationStageFlags) const noexcept
{
	VkCommandBuffer commandBuffer{ CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY) };

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		std::vector<VkFormat> depthFormatsWithStencil{
				VK_FORMAT_D32_SFLOAT_S8_UINT,
				VK_FORMAT_D24_UNORM_S8_UINT,
				VK_FORMAT_D16_UNORM_S8_UINT,
		};

		for (auto& depthFormat : depthFormatsWithStencil) {
			if (depthFormat == imageFormat) {
				barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
		}

	} else {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	// The srcAccessMask of the image memory barrier shows which operation
	// must be completed using the old layout, before the transition to the
	// new one happens.
	switch (oldLayout) {
		case VK_IMAGE_LAYOUT_UNDEFINED:
			barrier.srcAccessMask = 0;
			break;
		case VK_IMAGE_LAYOUT_PREINITIALIZED:
			barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		default:
			ERROR_LOG("Image layout transition failed: Initial layout not supported.");
			return false;
	}

	// Destination access mask controls the dependency for the new image layout
	switch (newLayout) {
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			if (barrier.srcAccessMask == 0) {
				barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
			}

			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		default:
			ERROR_LOG("Image layout transition failed: Target layout not supported.");
			return false;
	}

	VkCommandBufferBeginInfo commandBufferBeginInfo{};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

	// Put barrier inside setup command buffer
	vkCmdPipelineBarrier(commandBuffer,
	                     sourceStageFlags,
	                     destinationStageFlags,
	                     0,
	                     0,
	                     nullptr,
	                     0,
	                     nullptr,
	                     1,
	                     &barrier);

	vkEndCommandBuffer(commandBuffer);

	VkFenceCreateInfo fenceCreateInfo{};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_NULL_HANDLE;

	VkFence fence{ VK_NULL_HANDLE };

	VkResult result{ vkCreateFence(m_LogicalDevice, &fenceCreateInfo, nullptr, &fence) };

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create fence.");
		return false;
	}

	if (!SubmitCommandBuffer(commandBuffer, m_TransferQueue, fence)) {
		ERROR_LOG("Command buffer submission failed.");
		return false;
	}

	return true;
}

VkQueue VulkanDevice::GetQueue(QueueFamily queueFamily) const noexcept
{
	switch (queueFamily) {
		case QueueFamily::GRAPHICS:
		case QueueFamily::PRESENT:
			return m_GraphicsQueue;
		case QueueFamily::TRANSFER:
			return m_TransferQueue;
		case QueueFamily::COMPUTE:
			return m_ComputeQueue;
		default:
			return nullptr;
	}
}

VulkanDevice::operator VkDevice() const noexcept
{
	return m_LogicalDevice;
}

