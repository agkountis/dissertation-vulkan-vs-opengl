#include <vector>
#include "vulkan_framebuffer.h"
#include "logger.h"
#include "vulkan_infrastructure_context.h"

VulkanFramebuffer::~VulkanFramebuffer()
{
	Destroy();
}

bool VulkanFramebuffer::Create(const std::vector<VkImageView>& imageViews,
                               const Vec2ui& size,
                               VkRenderPass renderPass) noexcept
{
	VkFramebufferCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	createInfo.width = size.x;
	createInfo.height = size.y;
	createInfo.attachmentCount = static_cast<ui32>(imageViews.size());
	createInfo.pAttachments = imageViews.data();
	createInfo.renderPass = renderPass;
	createInfo.layers = 1;

	VkResult result{ vkCreateFramebuffer(G_VulkanDevice, &createInfo, nullptr, &m_Framebuffer) };

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create framebuffer.");
		return false;
	}

	return true;
}

void VulkanFramebuffer::Destroy() const noexcept
{
	vkDestroyFramebuffer(G_VulkanDevice, m_Framebuffer, nullptr);
}

VulkanFramebuffer::operator VkFramebuffer() const noexcept
{
	return m_Framebuffer;
}
