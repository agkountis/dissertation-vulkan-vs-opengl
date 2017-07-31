#include <cassert>
#include "vulkan_buffer.h"
#include "vulkan_infrastructure_context.h"
#include <cstring>

VulkanBuffer::~VulkanBuffer()
{
	LOG("Cleaning up VulkanBuffer");
	CleanUp();
}

VkResult VulkanBuffer::Map(VkDeviceSize size, VkDeviceSize offset) noexcept
{
	return vkMapMemory(G_VulkanDevice, memory, offset, size, 0, &data);
}

void VulkanBuffer::Unmap()
{
	if (data) {
		vkUnmapMemory(G_VulkanDevice, memory);
		data = nullptr;
	}
}

VkResult VulkanBuffer::Bind(VkDeviceSize offset) const noexcept
{
	return vkBindBufferMemory(G_VulkanDevice, buffer, memory, offset);
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
		vkDestroyBuffer(G_VulkanDevice, buffer, nullptr);
	}

	if (memory) {
		vkFreeMemory(G_VulkanDevice, memory, nullptr);
	}
}
