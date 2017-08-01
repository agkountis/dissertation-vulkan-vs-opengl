#ifndef VULKAN_INFRASTRUCTURE_CONTEXT_H_
#define VULKAN_INFRASTRUCTURE_CONTEXT_H_

#include <resource_manager.h>
#include "vulkan_application.h"

class VulkanInfrastructureContext {
private:
	static VulkanApplication* s_VulkanApplication;

public:
	static void RegisterApplication(VulkanApplication* application) noexcept;

	static VulkanApplication* GetApplication() noexcept;
};

#define G_VulkanInstance VulkanInfrastructureContext::GetApplication()->GetVulkanInstance()
#define G_VulkanDevice VulkanInfrastructureContext::GetApplication()->GetVulkanDevice()
#define G_VulkanDebug VulkanInfrastructureContext::GetApplication()->GetVulkanDebug()
#define G_ResourceManager VulkanInfrastructureContext::GetApplication()->GetResourceManager()

#endif //VULKAN_INFRASTRUCTURE_CONTEXT_H_
