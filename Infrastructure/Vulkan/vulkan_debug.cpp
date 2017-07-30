#include "vulkan_debug.h"
#include "logger.h"
#include "vulkan_infrastructure_context.h"

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

bool VulkanDebug::Initialize() noexcept
{
	VkDebugReportCallbackCreateInfoEXT createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	createInfo.pfnCallback = DebugCallback;

	//Because vkCreateDebugReportCallbackEXT is an extension function....we have to look up its address ourselves.
	auto vkCreateDebugReportCallbackEXT =
			reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(
					vkGetInstanceProcAddr(G_VulkanInstance, "vkCreateDebugReportCallbackEXT")
			);

	if (!vkCreateDebugReportCallbackEXT) {
		ERROR_LOG("Cannot setup debug callback. Extension not present.");
		return false;
	}

	VkResult result{ vkCreateDebugReportCallbackEXT(G_VulkanInstance, &createInfo, nullptr, &m_DebugReportCallback) };

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create debug report callback.");
		return false;
	}

	LOG("Successfully created debug report callback.");
	return true;
}

VulkanDebug::~VulkanDebug()
{
	auto vkDestroyDebugReportCallbackEXT =
			reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(
					vkGetInstanceProcAddr(G_VulkanInstance, "vkDestroyDebugReportCallbackEXT")
			);

	if (vkDestroyDebugReportCallbackEXT) {
		vkDestroyDebugReportCallbackEXT(G_VulkanInstance, m_DebugReportCallback, nullptr);
	}
}
