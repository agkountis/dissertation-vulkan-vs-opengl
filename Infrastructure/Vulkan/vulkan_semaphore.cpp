#include "vulkan_semaphore.h"
#include "logger.h"
#include "vulkan_infrastructure_context.h"

VulkanSemaphore::~VulkanSemaphore()
{
	LOG("Cleaning up VulkanSemaphore");
	Destroy();
}

bool VulkanSemaphore::Create() noexcept
{
	VkSemaphoreCreateInfo semaphoreCreateInfo{};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkResult result{ vkCreateSemaphore(G_VulkanDevice, &semaphoreCreateInfo, nullptr, &m_Semaphore) };

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create semaphore");
		return false;
	}

	return true;
}

void VulkanSemaphore::Destroy() const noexcept
{
	vkDestroySemaphore(G_VulkanDevice, m_Semaphore, nullptr);
}

VulkanSemaphore::operator VkSemaphore() const noexcept
{
	return m_Semaphore;
}

const VkSemaphore* VulkanSemaphore::Get() const noexcept
{
	return &m_Semaphore;
}
