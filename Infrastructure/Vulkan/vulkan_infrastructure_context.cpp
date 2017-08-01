#include "vulkan_infrastructure_context.h"

VulkanApplication* VulkanInfrastructureContext::s_VulkanApplication{ nullptr };

void VulkanInfrastructureContext::RegisterApplication(VulkanApplication* application) noexcept
{
	s_VulkanApplication = application;
}

VulkanApplication* VulkanInfrastructureContext::GetApplication() noexcept
{
	return s_VulkanApplication;
}
