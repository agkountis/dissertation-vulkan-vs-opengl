#ifndef VULKAN_INFRASTRUCTURE_CONTEXT_H_
#define VULKAN_INFRASTRUCTURE_CONTEXT_H_

#include "resource_manager.h"
#include "vulkan_instance.h"
#include "vulkan_device.h"

class VulkanInfrastructureContext {
private:
	static VulkanInstance* s_pInstance;

	static VulkanDevice* s_pDevice;

	static ResourceManager* s_pResourceManager;

public:
	static VulkanInstance& GetVulkanInstance() noexcept;

	static VulkanDevice& GetVulkanDevice() noexcept;

	static ResourceManager& GetResourceManager() noexcept;

	static void Register(VulkanInstance* instance,
	                     VulkanDevice* device,
	                     ResourceManager* resourceManager) noexcept;
};

#define G_VulkanInstance VulkanInfrastructureContext::GetVulkanInstance()
#define G_VulkanDevice VulkanInfrastructureContext::GetVulkanDevice()
#define G_ResourceManager VulkanInfrastructureContext::GetResourceManager()

#endif //VULKAN_INFRASTRUCTURE_CONTEXT_H_
