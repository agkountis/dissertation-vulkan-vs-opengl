#ifndef VULKAN_DEBUG_H_
#define VULKAN_DEBUG_H_

#include "vulkan_instance.h"

class VulkanDebug {
private:
	VkDebugReportCallbackEXT m_DebugReportCallback{VK_NULL_HANDLE};

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugReportFlagsEXT flags,
														VkDebugReportObjectTypeEXT objType,
														uint64_t obj,
														size_t location,
														int32_t code,
														const char *layerPrefix,
														const char *msg,
														void *userData);

public:
	bool Initialize() noexcept;

	~VulkanDebug();
};

#endif // VULKAN_DEBUG_H_
