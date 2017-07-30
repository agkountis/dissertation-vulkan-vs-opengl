#ifndef VULKAN_INFRASTRUCTURE_CONTEXT_H_
#define VULKAN_INFRASTRUCTURE_CONTEXT_H_

#include <resource_manager.h>
#include "vulkan_device.h"
#include "vulkan_instance.h"

#if !defined(NDEBUG) && !defined(__APPLE__)
#include "vulkan_debug.h"
#endif

class VulkanInfrastructureContext {
private:
	static VulkanInstance s_Instance;

	static VulkanDevice s_Device;

#if !defined(NDEBUG) && !defined(__APPLE__)
	static VulkanDebug s_VulkanDebug;
#endif

	static ResourceManager s_ResourceManager;

public:
	static VulkanInstance& GetVulkanInstance() noexcept;

	static VulkanDevice& GetVulkanDevice() noexcept;

#if !defined(NDEBUG) && !defined(__APPLE__)
	static VulkanDebug& GetVulkanDebug() noexcept;
#endif

	static ResourceManager& GetResourceManager() noexcept;
};

#define G_VulkanInstance VulkanInfrastructureContext::GetVulkanInstance()
#define G_VulkanDevice VulkanInfrastructureContext::GetVulkanDevice()
//#define G_VulkanDebug VulkanInfrastructureContext::GetVulkanDebug()
#define G_ResourceManager VulkanInfrastructureContext::GetResourceManager()

#endif //VULKAN_INFRASTRUCTURE_CONTEXT_H_
