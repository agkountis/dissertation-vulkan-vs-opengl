#include <cassert>
#include "vulkan_buffer.h"
#include <cstring>

VulkanBuffer::~VulkanBuffer()
{
	CleanUp();
}

VkResult VulkanBuffer::Map(VkDeviceSize size, VkDeviceSize offset) noexcept
{
	return vkMapMemory(pLogicalDevice, memory, offset, size, 0, &data);
}

void VulkanBuffer::Unmap()
{
	if (data) {
		vkUnmapMemory(pLogicalDevice, memory);
		data = nullptr;
	}
}

VkResult VulkanBuffer::Bind(VkDeviceSize offset) const noexcept
{
	return vkBindBufferMemory(pLogicalDevice, buffer, memory, offset);
}

void VulkanBuffer::Fill(void* data, VkDeviceSize size) const noexcept
{
	assert(data);
	memcpy(this->data, data, size);
}

void VulkanBuffer::InitializeDescriptor(VkDeviceSize range, VkDeviceSize offset) noexcept
{
	descriptorBufferInfo.buffer = buffer;
	descriptorBufferInfo.range = range;
	descriptorBufferInfo.offset = offset;
}

void VulkanBuffer::CleanUp() const noexcept
{
	if (buffer) {
		vkDestroyBuffer(pLogicalDevice, buffer, nullptr);
	}

	if (memory) {
		vkFreeMemory(pLogicalDevice, memory, nullptr);
	}
}
