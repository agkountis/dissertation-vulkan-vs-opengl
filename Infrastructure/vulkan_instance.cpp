#include "vulkan_instance.h"
#include "types.h"
#include "logger.h"

VulkanInstance::VulkanInstance(const VkApplicationInfo& applicationInfo,
                               const std::vector<const char*>& extensions,
                               const std::vector<const char*>& layers)
{
	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &applicationInfo;
	createInfo.enabledExtensionCount = extensions.size();
	createInfo.ppEnabledExtensionNames = extensions.data();

	if (!layers.empty()) {

		ui32 layerCount{ 0 };

		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> layerProperties{ layerCount };

		vkEnumerateInstanceLayerProperties(&layerCount, layerProperties.data());

		bool layerFound{ false };

		for (const auto& layerName : layers) {

			for (const auto& layerProperty : layerProperties) {

				if (strcmp(layerName, layerProperty.layerName) == 0) {
					layerFound = true;
					break;
				}

			}

		}

		if (!layerFound) {
			throw std::runtime_error("The validation layers requested are not available.");
		}

		createInfo.enabledLayerCount = layers.size();
		createInfo.ppEnabledLayerNames = layers.data();
	} 
	else {
		createInfo.enabledLayerCount = 0;
	}

	VkResult result{ vkCreateInstance(&createInfo, nullptr, &m_Instance) };

	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Vulkan instance");
	}

	LOG("Successfully created the Vulkan instance");
}

VulkanInstance::~VulkanInstance()
{
	vkDestroyInstance(m_Instance, nullptr);
}

VkInstance VulkanInstance::GetHandle() const noexcept
{
	return m_Instance;
}
