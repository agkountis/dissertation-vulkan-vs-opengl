#ifndef VULKAN_RENDER_PASS_H_
#define VULKAN_RENDER_PASS_H_

#include <vulkan/vulkan.h>

class VulkanRenderPass {
private:
	VkDevice m_pLogicalDevice{ nullptr };

	VkRenderPass m_RenderPass{ VK_NULL_HANDLE };

public:
	virtual ~VulkanRenderPass();

	virtual bool Create(VkDevice logicalDevice,
	                    VkFormat colorFormat,
	                    VkFormat depthFormat) noexcept;

	operator VkRenderPass() const noexcept;
};

#endif //VULKAN_RENDER_PASS_H_
