#include "vulkan_instance.h"
#include "types.h"
#include "logger.h"

VulkanInstance::~VulkanInstance()
{
	LOG("Cleaning up VulkanInstance");
	vkDestroyInstance(m_Instance, nullptr);
}

bool VulkanInstance::Create(const VkApplicationInfo& applicationInfo,
                            const std::vector<const char*>& extensions,
                            const std::vector<const char*>& layers) noexcept
{
	VkInstanceCreateInfo instanceCreateInfo{};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &applicationInfo;

	if (!extensions.empty()) {
		instanceCreateInfo.enabledExtensionCount = static_cast<ui32>(extensions.size());
		instanceCreateInfo.ppEnabledExtensionNames = extensions.data();
	}

	if (!layers.empty()) {
		instanceCreateInfo.enabledLayerCount = static_cast<ui32>(layers.size());
		instanceCreateInfo.ppEnabledLayerNames = layers.data();
	}

	VkResult result{ vkCreateInstance(&instanceCreateInfo, nullptr, &m_Instance) };

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create Vulkan Instance.");
		return false;
	}

	m_Extensions = extensions;

	m_Layers = layers;

	LOG("Successfully created Vulkan Instance.");
	return true;
}

VulkanInstance::operator VkInstance() const noexcept
{
	return m_Instance;
}
