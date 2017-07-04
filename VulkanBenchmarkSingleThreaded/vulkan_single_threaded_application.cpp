#include "vulkan_single_threaded_application.h"


bool VulkanSingleThreadedApplication::Initialize() noexcept
{
	if (!VulkanApplication::Initialize()) {
		return false;
	}



	return true;
}

void VulkanSingleThreadedApplication::Draw() const noexcept
{
}
