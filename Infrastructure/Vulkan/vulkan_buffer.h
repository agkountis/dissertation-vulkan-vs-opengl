#ifndef VULKAN_BUFFER_H_
#define VULKAN_BUFFER_H_

#include <vulkan/vulkan.h>

struct VulkanBuffer {
	VkBuffer buffer{ VK_NULL_HANDLE };

	VkDeviceMemory memory{ VK_NULL_HANDLE };

	VkDescriptorBufferInfo descriptorBufferInfo{};

	VkDeviceSize size{ VK_NULL_HANDLE };

	VkDeviceSize memoryAlignment{ VK_NULL_HANDLE };

	VkBufferUsageFlags usageFlags;

	VkMemoryPropertyFlags memoryPropertyFlags;

	void* data{ nullptr };

	~VulkanBuffer();

	VkResult Map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) noexcept;

	void Unmap();

	VkResult Bind(VkDeviceSize offset = 0) const noexcept;

	void Fill(void* data, VkDeviceSize size) const noexcept;

	void InitializeDescriptor(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) noexcept;

	void CleanUp() const noexcept;
};

#endif //VULKAN_BUFFER_H_
