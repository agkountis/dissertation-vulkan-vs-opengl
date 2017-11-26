#include "vulkan_render_target.h"
#include <cassert>
#include "vulkan_infrastructure_context.h"
#include <algorithm>
#include <array>

// VulkanRenderTargetAttachment -----------------------------------------------------------------
VulkanRenderTargetAttachment::VulkanRenderTargetAttachment(const Vec2ui& size,
                                                           const ui32 layerCount,
                                                           const VkFormat format,
                                                           const AttachmentType attachmentType,
                                                           const bool samplingEnabled)
	: m_Format{ format },
	  m_Size{ size },
	  m_LayerCount{ layerCount },
	  m_AttachmentType{ attachmentType },
	  m_SamplingEnabled{ samplingEnabled }
{
	assert(format != VK_FORMAT_UNDEFINED);
	assert(layerCount > 0);

	m_LayerCount = layerCount;
	m_Format = format;
}

VulkanRenderTargetAttachment::~VulkanRenderTargetAttachment()
{
	vkDestroyImage(G_VulkanDevice, m_Image, nullptr);
	vkDestroyImageView(G_VulkanDevice, m_ImageView, nullptr);
	vkFreeMemory(G_VulkanDevice, m_Memory, nullptr);
}

bool VulkanRenderTargetAttachment::HasDepth() const
{
	std::vector<VkFormat> formats{
		VK_FORMAT_D16_UNORM,
		VK_FORMAT_X8_D24_UNORM_PACK32,
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D16_UNORM_S8_UINT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D32_SFLOAT_S8_UINT,
	};

	return std::find(formats.begin(), formats.end(), m_Format) != std::end(formats);
}

bool VulkanRenderTargetAttachment::HasStencil() const noexcept
{
	std::vector<VkFormat> formats{
		VK_FORMAT_S8_UINT,
		VK_FORMAT_D16_UNORM_S8_UINT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D32_SFLOAT_S8_UINT,
	};

	return std::find(formats.begin(), formats.end(), m_Format) != std::end(formats);
}

bool VulkanRenderTargetAttachment::Create() noexcept
{
	VkImageAspectFlags aspectFlags{ VK_NULL_HANDLE };
	VkImageUsageFlags imageUsageFlags{ VK_NULL_HANDLE };

	switch (m_AttachmentType) {
	case AttachmentType::COLOR:
		aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
		imageUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		break;
	case AttachmentType::DEPTH:
		if (HasDepth()) {
			aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
		}

		if (HasStencil()) {
			aspectFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}

		assert(aspectFlags & VK_IMAGE_ASPECT_DEPTH_BIT);

		imageUsageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		break;
	}

	if (m_SamplingEnabled) {
		imageUsageFlags |= VK_IMAGE_USAGE_SAMPLED_BIT;
	}

	assert(aspectFlags);
	assert(imageUsageFlags);

	// Create the image
	VkImageCreateInfo imageCreateInfo{};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = m_Format;
	imageCreateInfo.extent = VkExtent3D{ m_Size.x, m_Size.y, 1 /* depth */ };
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = m_LayerCount;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = imageUsageFlags;

	VkResult result{ vkCreateImage(G_VulkanDevice, &imageCreateInfo, nullptr, &m_Image) };

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create image for render target attachment.");
		return false;
	}

	// Allocate memory for the image.
	VkMemoryAllocateInfo memoryAllocateInfo{};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

	const VkMemoryRequirements memoryRequirements{ G_VulkanDevice.GetImageMemoryRequirements(m_Image) };

	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = G_VulkanDevice.GetMemoryTypeIndex(memoryRequirements.memoryTypeBits,
	                                                                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	result = vkAllocateMemory(G_VulkanDevice, &memoryAllocateInfo, nullptr, &m_Memory);

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to allocate memory for render target attachment image.");
	}

	// Bind the memory to the image
	result = vkBindImageMemory(G_VulkanDevice, m_Image, m_Memory, 0);

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to bind memory for render target attachment image.");
		return false;
	}

	m_SubresourceRange.aspectMask = aspectFlags;
	m_SubresourceRange.levelCount = 1;
	m_SubresourceRange.layerCount = m_LayerCount;

	// Create the image view.
	VkImageViewCreateInfo imageViewCreateInfo{};
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

	if (m_LayerCount == 1) {
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	}
	else {
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
	}

	imageViewCreateInfo.format = m_Format;
	imageViewCreateInfo.subresourceRange = m_SubresourceRange;
	imageViewCreateInfo.subresourceRange.aspectMask = aspectFlags;
	imageViewCreateInfo.image = m_Image;

	result = vkCreateImageView(G_VulkanDevice, &imageViewCreateInfo, nullptr, &m_ImageView);

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create image view for render target attachment.");
		return false;
	}

	m_Description.samples = VK_SAMPLE_COUNT_1_BIT;
	m_Description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;

	// If the attachment is going to be sampled...
	if (m_SamplingEnabled) {
		m_Description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	}
	else {
		// don't care!
		m_Description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	}

	// No1 cares about stencil...
	m_Description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	m_Description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	m_Description.format = m_Format;
	m_Description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	if (m_AttachmentType == AttachmentType::DEPTH) {
		m_Description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	}
	else {
		m_Description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}

	return true;
}

VkAttachmentDescription VulkanRenderTargetAttachment::GetDescription() const noexcept
{
	return m_Description;
}

VkImageView VulkanRenderTargetAttachment::GetImageView() const noexcept
{
	return m_ImageView;
}

ui32 VulkanRenderTargetAttachment::GetLayerCount() const noexcept
{
	return m_LayerCount;
}

// ----------------------------------------------------------------------------------------------


// VulkanRenderTarget ---------------------------------------------------------------------------

bool VulkanRenderTarget::CreateSampler(const VkFilter magFilter,
                                       const VkFilter minFilter,
                                       const VkSamplerAddressMode addressMode) noexcept
{
	VkSamplerCreateInfo samplerCreateInfo{};
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.magFilter = magFilter;
	samplerCreateInfo.minFilter = minFilter;
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCreateInfo.addressModeU = addressMode;
	samplerCreateInfo.addressModeV = addressMode;
	samplerCreateInfo.addressModeW = addressMode;
	samplerCreateInfo.mipLodBias = 0.0f;
	samplerCreateInfo.maxAnisotropy = 1.0f;
	samplerCreateInfo.minLod = 0.0f;
	samplerCreateInfo.maxLod = 1.0f;
	samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

	const VkResult result{ vkCreateSampler(G_VulkanDevice, &samplerCreateInfo, nullptr, &m_Sampler) };

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create texture sampler for render target.");
		return false;
	}

	return true;
}

VulkanRenderTarget::~VulkanRenderTarget()
{
	vkDestroySampler(G_VulkanDevice, m_Sampler, nullptr);
	vkDestroyRenderPass(G_VulkanDevice, m_RenderPass, nullptr);
}

bool VulkanRenderTarget::Create(const Vec2ui& size) noexcept
{
	if (m_Attachments.empty()) {
		ERROR_LOG("Cannot create render pass with 0 attachments.");
		return false;
	}

	m_Size = size;

	std::vector<VkAttachmentDescription> attachmentDescriptions;

	std::vector<VkAttachmentReference> colorAttachmentReferences;
	VkAttachmentReference depthAttachmentReference{};

	bool hasDepthAttachment{ false };
	for (auto i = 0; i < m_Attachments.size(); ++i) {

		if (!m_Attachments[i].Create()) {
			ERROR_LOG("Attachment creation failed. Attachment index: " + i);
			return false;
		}

		attachmentDescriptions.push_back(m_Attachments[i].GetDescription());

		if ((m_Attachments[i].HasDepth() ||
			m_Attachments[i].HasStencil()) && !hasDepthAttachment) {
			depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			depthAttachmentReference.attachment = i;

			hasDepthAttachment = true;
		}
		else {
			VkAttachmentReference attachmentReference{};
			attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			attachmentReference.attachment = i;

			colorAttachmentReferences.push_back(attachmentReference);
		}
	}

	VkSubpassDescription subpassDescription{};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.pColorAttachments = colorAttachmentReferences.data();
	subpassDescription.colorAttachmentCount = static_cast<ui32>(colorAttachmentReferences.size());

	if (hasDepthAttachment) {
		subpassDescription.pDepthStencilAttachment = &depthAttachmentReference;
	}

	std::array<VkSubpassDependency, 2> subpassDependencies;
	subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependencies[0].dstSubpass = 0;
	subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	subpassDependencies[1].srcSubpass = 0;
	subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpassDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	// Create render pass
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pAttachments = attachmentDescriptions.data();
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpassDescription;
	renderPassInfo.dependencyCount = static_cast<ui32>(subpassDependencies.size());
	renderPassInfo.pDependencies = subpassDependencies.data();

	VkResult result{ vkCreateRenderPass(G_VulkanDevice, &renderPassInfo, nullptr, &m_RenderPass) };

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create renderpass for the render target.");
		return false;
	}

	std::vector<VkImageView> attachmentViews;
	ui32 maxLayerCount{ 0 };
	for (const auto& attachment : m_Attachments) {
		attachmentViews.push_back(attachment.GetImageView());

		if (attachment.GetLayerCount() > maxLayerCount) {
			maxLayerCount = attachment.GetLayerCount();
		}
	}

	VkFramebufferCreateInfo framebufferInfo = {};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = m_RenderPass;
	framebufferInfo.pAttachments = attachmentViews.data();
	framebufferInfo.attachmentCount = static_cast<uint32_t>(attachmentViews.size());
	framebufferInfo.width = m_Size.x;
	framebufferInfo.height = m_Size.y;
	framebufferInfo.layers = maxLayerCount;

	result = vkCreateFramebuffer(G_VulkanDevice, &framebufferInfo, nullptr, &m_Framebuffer);

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create framebuffer for the render target");
		return false;
	}

	return true;
}

bool VulkanRenderTarget::Create(const Vec2ui& size,
                                const VkFilter magFilter,
                                const VkFilter minFilter,
                                const VkSamplerAddressMode addressMode) noexcept
{
	if (!Create(size)) {
		return false;
	}

	if (!CreateSampler(magFilter, minFilter, addressMode)) {
		ERROR_LOG("Failed to create texture sampler for the render target.");
		return false;
	}

	return true;
}

size_t VulkanRenderTarget::AddAttachment(const Vec2ui& size,
                                         const ui32 layerCount,
                                         const VkFormat format,
                                         const AttachmentType attachmentType,
                                         const bool samplingEnabled) noexcept
{
	m_Attachments.push_back(VulkanRenderTargetAttachment{
		size,
		layerCount,
		format,
		attachmentType,
		samplingEnabled
	});

	return m_Attachments.size() - 1;
}

const VulkanRenderTargetAttachment& VulkanRenderTarget::GetAttachment(const size_t index) const noexcept
{
	assert(index >= 0 && index < m_Attachments.size());
	return m_Attachments[index];
}

VkRenderPass VulkanRenderTarget::GetRenderPass() const noexcept
{
	return m_RenderPass;
}

const Vec2ui& VulkanRenderTarget::GetSize() const noexcept
{
	return m_Size;
}

VkSampler VulkanRenderTarget::GetSampler() const noexcept
{
	return m_Sampler;
}

VulkanRenderTarget::operator VkFramebuffer() const
{
	return m_Framebuffer;
}

// ----------------------------------------------------------------------------------------------
