#include <vector>
#include "vulkan_device.h"
#include "types.h"
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
			m_PhysicalDevice.supportedExtensions.push_back(std::string{ extension.extensionName });
		}
	}

	return true;
}
// -------------------------------------------------------------

VulkanDevice::~VulkanDevice()
{
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
                                       std::vector<const char *> extensionsToEnable,
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
	if (useSwapChain)
	{
		// If the device will be used for presenting to a display via a swapchain we need to request the swapchain extension
		deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	}

	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = static_cast<ui32>(queueCreateInfos.size());
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.pEnabledFeatures = &featuresToEnable;

	if (!deviceExtensions.empty())
	{
		deviceCreateInfo.enabledExtensionCount = static_cast<ui32>(deviceExtensions.size());
		deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
	}

	VkResult result{ vkCreateDevice(m_PhysicalDevice, &deviceCreateInfo, nullptr, &m_LogicalDevice) };

	if (result != VK_SUCCESS){
		ERROR_LOG("Failed to create logical device.");
		return false;
	}

	m_CommandPool = CreateCommandPool(m_QueueFamilyIndices.graphics);

	if (!m_CommandPool) {
		ERROR_LOG("Failed to create command pool");
		return false;
	}

	m_EnabledFeatures = featuresToEnable;

	return true;
}

const VulkanPhysicalDevice& VulkanDevice::GetPhysicalDevice() const noexcept
{
	return m_PhysicalDevice;
}

VkCommandPool VulkanDevice::CreateCommandPool(ui32 queueFamilyIndex, VkCommandPoolCreateFlags createFlags)
{
	VkCommandPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.queueFamilyIndex = queueFamilyIndex;
	createInfo.flags = createFlags;

	VkCommandPool commandPool{ nullptr };
	vkCreateCommandPool(m_LogicalDevice, &createInfo, nullptr, &commandPool);

	return commandPool;
}

VulkanDevice::operator VkDevice() noexcept
{
	return m_LogicalDevice;
}
