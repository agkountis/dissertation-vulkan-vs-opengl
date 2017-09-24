#include "vulkan_infrastructure_context.h"

VulkanInstance* VulkanInfrastructureContext::s_pInstance{ nullptr };

VulkanDevice* VulkanInfrastructureContext::s_pDevice{ nullptr };

ResourceManager* VulkanInfrastructureContext::s_pResourceManager{ nullptr };

VulkanApplication* VulkanInfrastructureContext::s_Application{ nullptr };

VulkanInstance& VulkanInfrastructureContext::GetVulkanInstance() noexcept
{
	return *s_pInstance;
}

VulkanDevice& VulkanInfrastructureContext::GetVulkanDevice() noexcept
{
	return *s_pDevice;
}

ResourceManager& VulkanInfrastructureContext::GetResourceManager() noexcept
{
	return *s_pResourceManager;
}

VulkanApplication& VulkanInfrastructureContext::GetApplication() noexcept
{
	return *s_Application;
}

void VulkanInfrastructureContext::Register(VulkanInstance* instance,
                                           VulkanDevice* device,
                                           ResourceManager* resourceManager, VulkanApplication* application) noexcept
{
	assert(instance);
	assert(device);
	assert(resourceManager);
	assert(application);

	s_pInstance = instance;

	s_pDevice = device;

	s_pResourceManager = resourceManager;

	s_Application = application;
}

