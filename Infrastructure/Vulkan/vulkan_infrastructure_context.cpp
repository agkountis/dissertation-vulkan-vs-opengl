#include "vulkan_infrastructure_context.h"

VulkanInstance VulkanInfrastructureContext::s_Instance;

VulkanDevice VulkanInfrastructureContext::s_Device;

#if !defined(NDEBUG) && !defined(__APPLE__)
VulkanDebug VulkanInfrastructureContext::s_VulkanDebug;
#endif

ResourceManager VulkanInfrastructureContext::s_ResourceManager;

VulkanInstance& VulkanInfrastructureContext::GetVulkanInstance() noexcept
{
	return s_Instance;
}

VulkanDevice& VulkanInfrastructureContext::GetVulkanDevice() noexcept
{
	return s_Device;
}

#if !defined(NDEBUG) && !defined(__APPLE__)
VulkanDebug& VulkanInfrastructureContext::GetVulkanDebug() noexcept
{
	return s_VulkanDebug;
}
#endif

ResourceManager& VulkanInfrastructureContext::GetResourceManager() noexcept
{
	return s_ResourceManager;
}

