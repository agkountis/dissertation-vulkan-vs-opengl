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

	Vec2ui m_Size;

	VkDescriptorImageInfo m_Descriptor{};

public:
	explicit VulkanTexture(TextureType textureType,
	                       VkFormat format,
	                       VkImageAspectFlagBits imageAspectFlagBits);

	~VulkanTexture() override;

	bool Load(const std::string& fileName) noexcept override;
};

#endif //VULKAN_TEXTURE_H_
