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

	bool Create() noexcept;

	bool HasDepth() const;

	bool HasStencil() const noexcept;

	VkAttachmentDescription GetDescription() const noexcept;

	VkImageView GetImageView() const noexcept;

	ui32 GetLayerCount() const noexcept;

	VkImage GetImage() const noexcept;
};

class VulkanRenderTarget {
private:
	std::vector<VulkanRenderTargetAttachment> m_Attachments;

	Vec2ui m_Size;

	VkFramebuffer m_Framebuffer{ VK_NULL_HANDLE };

	VkRenderPass m_RenderPass{ VK_NULL_HANDLE };

	VkSampler m_Sampler{ VK_NULL_HANDLE };

	bool CreateSampler(const VkFilter magFilter,
	                   const VkFilter minFilter,
	                   const VkSamplerAddressMode addressMode) noexcept;

public:
	~VulkanRenderTarget();

	bool Create(const Vec2ui& size) noexcept;

	bool Create(const Vec2ui& size, const VkFilter magFilter,
	            const VkFilter minFilter,
	            const VkSamplerAddressMode addressMode) noexcept;

	size_t AddAttachment(const Vec2ui& size,
	                     const ui32 layerCount,
	                     const VkFormat format,
	                     const AttachmentType attachmentType,
	                     const bool samplingEnabled) noexcept;

	const VulkanRenderTargetAttachment& GetAttachment(const size_t index) const noexcept;

	VkRenderPass GetRenderPass() const noexcept;

	const Vec2ui& GetSize() const noexcept;

	VkSampler GetSampler() const noexcept;

	operator VkFramebuffer() const;
};

#endif //DISSERTATION_VULKAN_RENDER_TARGET_H_
