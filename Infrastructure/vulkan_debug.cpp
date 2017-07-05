#include "vulkan_debug.h"
#include "logger.h"

VkBool32 VulkanDebug::DebugCallback(VkDebugReportFlagsEXT flags,
                                    VkDebugReportObjectTypeEXT objType,
                                    uint64_t obj,
                                    size_t location,
                                    int32_t code,
                                    const char* layerPrefix,
                                    const char* msg,
                                    void* userData)
{
	ERROR_LOG("Validation layer: " + std::string{ msg });

	return VK_FALSE;
}

bool VulkanDebug::Initialize(VkInstance instance) noexcept
{
	if (!instance) {
		ERROR_LOG("Could not initialize VulkanDebug. VkInstance was nullptr");
		return false;
	}

	VkDebugReportCallbackCreateInfoEXT createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	createInfo.pfnCallback = DebugCallback;

	m_InstanceHandle = instance;

	//Because vkCreateDebugReportCallbackEXT is an extension function....we have to look up its address ourselves.
	auto vkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(m_InstanceHandle, "vkCreateDebugReportCallbackEXT"));

	if (!vkCreateDebugReportCallbackEXT) {
		ERROR_LOG("Cannot setup debug callback. Extension not present.");
		return false;
	}

	VkResult result{ vkCreateDebugReportCallbackEXT(m_InstanceHandle, &createInfo, nullptr, &m_DebugReportCallback) };

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create debug report callback.");
		return false;
	}

	LOG("Successfully created debug report callback.");
	return true;
}

VulkanDebug::~VulkanDebug()
{
	auto vkDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(m_InstanceHandle, "vkCreateDebugReportCallbackEXT"));

	if (vkDestroyDebugReportCallbackEXT) {
		vkDestroyDebugReportCallbackEXT(m_InstanceHandle, m_DebugReportCallback, nullptr);
	}
}
