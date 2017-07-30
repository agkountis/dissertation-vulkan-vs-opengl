#ifndef VULKAN_FRAMEBUFFER_H_
#define VULKAN_FRAMEBUFFER_H_

#include <vulkan/vulkan.h>
#include "types.h"

class VulkanFramebuffer {
private:
	VkFramebuffer m_Framebuffer{ VK_NULL_HANDLE };

public:
	~VulkanFramebuffer();

	bool Create(const std::vector<VkImageView>& imageViews,
	            const Vec2ui& size,
	            VkRenderPass renderPass) noexcept;

	void Destroy() const noexcept;

	operator VkFramebuffer() const noexcept;
};

#endif //VULKAN_FRAMEBUFFER_H_
