#ifndef VULKAN_BUFFER_H_
#define VULKAN_BUFFER_H_

#include <vulkan/vulkan.h>

struct VulkanBuffer {
	VkDevice pLogicalDevice{ nullptr };

	VkBuffer buffer{ VK_NULL_HANDLE };

	VkDeviceMemory memory{ VK_NULL_HANDLE };

	VkDescriptorBufferInfo descriptor;

	VkDeviceSize size{ VK_NULL_HANDLE };

	VkDeviceSize memoryAlignment{ VK_NULL_HANDLE };

	VkBufferUsageFlags usageFlags;

	VkMemoryPropertyFlags memoryPropertyFlags;

	void* data{ nullptr };

	~VulkanBuffer();

	VkResult Map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) noexcept;

	void Unmap();

	VkResult Bind(VkDeviceSize offset = 0) noexcept;

	void Fill(void* data, VkDeviceSize size) noexcept;

	void InitializeDescriptor(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) noexcept;

	void CleanUp() noexcept;

};

#endif //VULKAN_BUFFER_H_
