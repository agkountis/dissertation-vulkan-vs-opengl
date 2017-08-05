#ifndef VULKAN_TEXTURE_H_
#define VULKAN_TEXTURE_H_

#include <vulkan/vulkan.h>
#include "texture.h"
#include "vulkan_device.h"

class VulkanTexture : public Texture {
private:
	VkImage m_Image{ VK_NULL_HANDLE };

	VkDeviceMemory m_ImageMemory{ VK_NULL_HANDLE };

	VkImageView m_ImageView{ VK_NULL_HANDLE };

	VkFormat m_Format{ VK_FORMAT_UNDEFINED };

	VkImageAspectFlags m_ImageAspectFlags{ VK_NULL_HANDLE };

	VkImageLayout m_ImageLayout{ VK_IMAGE_LAYOUT_PREINITIALIZED };

	Vec2ui m_Size;

public:
	explicit VulkanTexture(TextureType textureType,
	                       VkFormat format,
	                       VkImageAspectFlagBits imageAspectFlagBits);

	~VulkanTexture() override;

	VkImageView GetImageView() const noexcept;

	VkImageLayout GetImageLayout() const noexcept;

	bool Load(const std::string& fileName) noexcept override;
};

#endif //VULKAN_TEXTURE_H_
