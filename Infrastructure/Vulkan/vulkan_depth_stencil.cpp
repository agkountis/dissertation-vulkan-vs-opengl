#include "vulkan_depth_stencil.h"
#include "logger.h"
#include "vulkan_device.h"
#include "vulkan_infrastructure_context.h"


VulkanDepthStencil::~VulkanDepthStencil()
{
	LOG("Cleaning up VulkanDepthStencil");
	Destroy();
}

VkImageView VulkanDepthStencil::GetImageView() const noexcept
{
	return m_ImageView;
}

VkImage VulkanDepthStencil::GetImage() const noexcept
{
	return m_Image;
}

VkFormat VulkanDepthStencil::GetFormat() const noexcept
{
	return m_Format;
}

bool VulkanDepthStencil::Create(const Vec2ui& size, VkFormat format) noexcept
{
	VkImageCreateInfo imageCreateInfo{};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = format;
	imageCreateInfo.extent = VkExtent3D{ size.x, size.y, 1 };
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	imageCreateInfo.flags = 0;

	VkImageViewCreateInfo depthStencilView{};
	depthStencilView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	depthStencilView.viewType = VK_IMAGE_VIEW_TYPE_2D;
	depthStencilView.format = format;
	depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	depthStencilView.subresourceRange.baseMipLevel = 0;
	depthStencilView.subresourceRange.levelCount = 1;
	depthStencilView.subresourceRange.baseArrayLayer = 0;
	depthStencilView.subresourceRange.layerCount = 1;

	VkResult result{ vkCreateImage(G_VulkanDevice, &imageCreateInfo, nullptr, &m_Image) };

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create depth image.");
		return false;
	}

	VkMemoryRequirements memoryRequirements{};
	vkGetImageMemoryRequirements(G_VulkanDevice, m_Image, &memoryRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo{};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = G_VulkanDevice.GetMemoryTypeIndex(memoryRequirements.memoryTypeBits,
	                                                                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	result = vkAllocateMemory(G_VulkanDevice, &memoryAllocateInfo, nullptr, &m_Memory);

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to allocate depth stencil m_Memory.");
		return false;
	}

	result = vkBindImageMemory(G_VulkanDevice, m_Image, m_Memory, 0);

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to bind depth image m_Memory.");
		return false;
	}

	depthStencilView.image = m_Image;

	result = vkCreateImageView(G_VulkanDevice, &depthStencilView, nullptr, &m_ImageView);

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create depth image view.");
		return false;
	}

	m_Format = format;

	return true;
}

void VulkanDepthStencil::Destroy() const noexcept
{
	vkDestroyImageView(G_VulkanDevice, m_ImageView, nullptr);

	vkDestroyImage(G_VulkanDevice, m_Image, nullptr);

	vkFreeMemory(G_VulkanDevice, m_Memory, nullptr);
}
