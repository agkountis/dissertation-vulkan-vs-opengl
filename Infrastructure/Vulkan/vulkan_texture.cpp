#include <logger.h>
#include "vulkan_texture.h"

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"
#include "vulkan_infrastructure_context.h"

VulkanTexture::VulkanTexture(TextureType textureType,
                             VkFormat format,
                             VkImageAspectFlagBits imageAspectFlagBits)
		: Texture{ textureType },
		  m_Format{ format }
{
	m_ImageAspectFlags |= imageAspectFlagBits;
}

VulkanTexture::~VulkanTexture()
{
	vkDestroyImageView(G_VulkanDevice, m_ImageView, nullptr);
	vkDestroyImage(G_VulkanDevice, m_Image, nullptr);
	vkFreeMemory(G_VulkanDevice, m_ImageMemory, nullptr);
}

bool VulkanTexture::Load(const std::string& fileName) noexcept
{
	Vec2i size;
	int colorChannels;

	stbi_uc* pixels{ stbi_load(fileName.c_str(),
	                           &size.x,
	                           &size.y,
	                           &colorChannels,
	                           STBI_rgb_alpha) };

	m_Size = Vec2ui{ size.x, size.y };

	VkDeviceSize imageSize{ static_cast<ui32>(size.x) * static_cast<ui32>(size.y) * 4 };

	if (!pixels) {
		ERROR_LOG("Failed to load image: " + fileName);
		return false;
	}

	VulkanBuffer stagingBuffer;

	if (!G_VulkanDevice.CreateBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	                             stagingBuffer,
	                             imageSize,
	                             pixels)) {
		ERROR_LOG("Failed to create staging buffer.");
		return false;
	}

	//Data got copied. No need to keep it around.
	stbi_image_free(pixels);

	// Create the image
	if (!G_VulkanDevice.CreateImage(m_Size,
	                            VK_FORMAT_R8G8B8A8_UNORM, //make this a texture class member.
	                            VK_IMAGE_TILING_OPTIMAL,
	                            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
	                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	                            m_Image,
	                            m_ImageMemory)) {
		ERROR_LOG("Failed to create image.");
		return false;
	}

	// transition the image from the default pre-initialized layout to
	// transfer destination optimal to copy the pixels from the staging buffer.
	if (!G_VulkanDevice.TransitionImageLayout(m_Image,
	                                      m_Format,
	                                      VK_IMAGE_LAYOUT_PREINITIALIZED,
	                                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)) {
		ERROR_LOG("Failed to transition image layout.");
		return false;
	}

	// Copy the data from the staging buffer to the image.
	if (!G_VulkanDevice.CopyBufferToImage(stagingBuffer,
	                                  m_Image,
	                                  m_Size)) {
		ERROR_LOG("Failed to copy buffer to image.");
		return false;
	}

	// Transition the image layout to shader read only optimal for optimal shader sampling.
	if (!G_VulkanDevice.TransitionImageLayout(m_Image,
	                                      m_Format,
	                                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	                                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)) {
		ERROR_LOG("Failed to transition image layout.");
		return false;
	}

	// Create the image view
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = m_Image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = m_Format;
	viewInfo.subresourceRange.aspectMask = m_ImageAspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkResult result{ vkCreateImageView(G_VulkanDevice, &viewInfo, nullptr, &m_ImageView) };

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create image view.");
		return false;
	}

	return true;
}
