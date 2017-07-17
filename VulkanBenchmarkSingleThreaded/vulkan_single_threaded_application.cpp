#include <iostream>
#include "vulkan_single_threaded_application.h"
#include "logger.h"

bool VulkanSingleThreadedApplication::CreatePipelines() noexcept
{
	return true;
}

VulkanSingleThreadedApplication::VulkanSingleThreadedApplication(const ApplicationSettings& settings)
		: VulkanApplication{ settings }
{
}


VulkanSingleThreadedApplication::~VulkanSingleThreadedApplication()
{
	vkDestroyPipeline(GetDevice(), m_Pipeline, nullptr);

	vkDestroyPipelineLayout(GetDevice(), m_PipelineLayout, nullptr);
}

bool VulkanSingleThreadedApplication::Initialize() noexcept
{
	if (!VulkanApplication::Initialize()) {
		return false;
	}

	return true;
}

void VulkanSingleThreadedApplication::Draw() const noexcept
{
	std::cout << "Drawing" << std::endl;
}

void VulkanSingleThreadedApplication::EnableFeatures() noexcept
{
	const auto& physicalDevice = GetDevice().GetPhysicalDevice();

	if (physicalDevice.features.samplerAnisotropy) {
		GetFeaturesToEnable().samplerAnisotropy = VK_TRUE;
	}
}
