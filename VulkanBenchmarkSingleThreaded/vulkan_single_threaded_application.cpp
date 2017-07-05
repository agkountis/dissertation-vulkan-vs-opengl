#include <iostream>
#include "vulkan_single_threaded_application.h"

VulkanSingleThreadedApplication::VulkanSingleThreadedApplication(const ApplicationSettings& settings)
		: VulkanApplication{ settings }
{
}

bool VulkanSingleThreadedApplication::Initialize() noexcept
{
	return VulkanApplication::Initialize();
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
