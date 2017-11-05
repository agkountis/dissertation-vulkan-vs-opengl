#ifndef DISSERTATION_VULKAN_RENDER_TARGET_H_
#define DISSERTATION_VULKAN_RENDER_TARGET_H_
#include <vulkan/vulkan.h>
#include <vector>
#include "types.h"

enum class AttachmentType {
	COLOR,
	DEPTH
};

class VulkanRenderTargetAttachment {
private:
	VkImage m_Image{ VK_NULL_HANDLE };

	VkDeviceMemory m_Memory{ VK_NULL_HANDLE };

	VkImageView m_ImageView{ VK_NULL_HANDLE };

	VkFormat m_Format{ VK_FORMAT_UNDEFINED };

	Vec2ui m_Size;

	ui32 m_LayerCount{ 0 };

	AttachmentType m_AttachmentType;

	bool m_SamplingEnabled{ false };

	VkImageSubresourceRange m_SubresourceRange{};

	VkAttachmentDescription m_Description{};

public:
	VulkanRenderTargetAttachment(const Vec2ui& size,
	                             const ui32 layerCount,
	                             const VkFormat format,
	                             const AttachmentType attachmentType,
	                             const bool samplingEnabled);

	~VulkanRenderTargetAttachment();

	bool HasDepth() const;

	bool HasStencil() const noexcept;

	bool Create() noexcept;

	VkAttachmentDescription GetDescription() const noexcept;

	VkImageView GetImageView() const noexcept;

	ui32 GetLayerCount() const noexcept;
};

class VulkanRenderTarget {
private:
	std::vector<VulkanRenderTargetAttachment> m_Attachments;

	Vec2ui m_Size;

	VkFramebuffer m_Framebuffer{ VK_NULL_HANDLE };

	VkRenderPass m_RenderPass{ VK_NULL_HANDLE };

	VkSampler m_Sampler{ VK_NULL_HANDLE };

public:
	~VulkanRenderTarget();

	bool CreateSampler(const VkFilter magFilter,
	                   const VkFilter minFilter,
	                   const VkSamplerAddressMode addressMode) noexcept;

	bool CreateRenderPass(bool overwriteExisting = false) noexcept;

	size_t AddAttachment(const VulkanRenderTargetAttachment&& attachment) noexcept;
};

#endif //DISSERTATION_VULKAN_RENDER_TARGET_H_
