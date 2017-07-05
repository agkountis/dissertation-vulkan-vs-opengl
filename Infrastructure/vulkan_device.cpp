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
	//TODO
	return true;
}

const VulkanPhysicalDevice& VulkanDevice::GetPhysicalDevice() const noexcept
{
	return m_PhysicalDevice;
}
