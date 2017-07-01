#ifndef VULKAN_DEBUG_H_
#define VULKAN_DEBUG_H_
#include "vulkan_instance.h"
#include <memory>

class VulkanDebug {
private:
	VkDebugReportCallbackEXT m_DebugReportCallback{ VK_NULL_HANDLE };

	VkInstance m_InstanceHandle{ nullptr };

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugReportFlagsEXT flags,
	                                                    VkDebugReportObjectTypeEXT objType,
	                                                    uint64_t obj,
	                                                    size_t location,
	                                                    int32_t code,
	                                                    const char* layerPrefix,
	                                                    const char* msg,
	                                                    void* userData);

public:
	explicit VulkanDebug(const std::unique_ptr<VulkanInstance>& instance);

	~VulkanDebug();
};

#endif // VULKAN_DEBUG_H_
