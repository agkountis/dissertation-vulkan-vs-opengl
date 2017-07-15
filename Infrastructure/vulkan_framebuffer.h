#ifndef VULKAN_FRAMEBUFFER_H_
#define VULKAN_FRAMEBUFFER_H_

#include <vulkan/vulkan.h>
#include "types.h"

class VulkanFramebuffer {
private:
	VkDevice m_pLogicalDevice{ nullptr };

	VkFramebuffer m_Framebuffer{ VK_NULL_HANDLE };

public:
	~VulkanFramebuffer();

	bool Create(VkDevice logicalDevice,
	            const std::vector<VkImageView>& imageViews,
	            const Vec2ui& size,
	            VkRenderPass renderPass) noexcept;

	operator VkFramebuffer() const noexcept;
};

#endif //VULKAN_FRAMEBUFFER_H_
