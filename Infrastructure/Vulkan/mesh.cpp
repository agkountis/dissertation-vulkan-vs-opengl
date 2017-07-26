#include <array>
#include "logger.h"
#include "vulkan_mesh.h"

VkVertexInputBindingDescription VulkanMesh::GetVertexInputBindingDescription() noexcept
{
	VkVertexInputBindingDescription vertexInputBindingDescription{};
	vertexInputBindingDescription.binding = 0;
	vertexInputBindingDescription.stride = sizeof(Vertex);
	vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return vertexInputBindingDescription;
}

std::array<VkVertexInputAttributeDescription, 5> VulkanMesh::GetVertexInputAttributeDescriptions() noexcept
{
	std::array<VkVertexInputAttributeDescription, 5> attributeDescriptions{};

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].offset = static_cast<ui32>(offsetof(Vertex, position));

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset = static_cast<ui32>(offsetof(Vertex, normal));

	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[2].offset = static_cast<ui32>(offsetof(Vertex, tangent));

	attributeDescriptions[3].binding = 0;
	attributeDescriptions[3].location = 3;
	attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[3].offset = static_cast<ui32>(offsetof(Vertex, color));

	attributeDescriptions[4].binding = 0;
	attributeDescriptions[4].location = 4;
	attributeDescriptions[4].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[4].offset = static_cast<ui32>(offsetof(Vertex, texcoord));

	return attributeDescriptions;
}

bool VulkanMesh::CreateBuffers(const VulkanDevice& device) noexcept
{
	VulkanBuffer stagingBuffer;

	ui64 vertexBufferSize{ static_cast<ui64>(sizeof(Vertex) * GetVertices().size()) };

	//Create and fill a staging buffer.
	//Used to save the data locally so it can be copied to device local memory for optimal performance.
	if (!device.CreateBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	                         stagingBuffer,
	                         vertexBufferSize,
	                         GetVertexDataPtr())) {
		ERROR_LOG("| VulkanMesh buffer creation failed:");
		ERROR_LOG("|-- Failed to create staging buffer.");
		return false;
	}

	//Create the vertex buffer with device local memory properties.
	//Additional mark the buffer a transfer destination so we can optimally copy data into it.
	if (!device.CreateBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
	                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	                         m_Vbo,
	                         vertexBufferSize,
	                         nullptr)) {
		ERROR_LOG("| VulkanMesh buffer creation failed:");
		ERROR_LOG("|-- Failed to create vertex buffer.");
		return false;
	}

	// Copy the staging buffer to the vertex buffer.
	if (!device.CopyBuffer(stagingBuffer, m_Vbo, device.GetQueue(QueueFamily::TRANSFER))) {
		ERROR_LOG("| VulkanMesh buffer creation failed:");
		ERROR_LOG("|-- Failed to transfer data from the staging buffer to the vertex buffer");
		return false;
	}

	ui64 indexBufferSize{ static_cast<ui64>(sizeof(ui32) * GetIndices().size()) };

	if (indexBufferSize) {
		//Cleanup the staging buffer to re-use it for the index buffer.
		stagingBuffer.CleanUp();

		// Re-create the staging buffer with the index data.
		if (!device.CreateBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		                         stagingBuffer,
		                         indexBufferSize,
		                         GetIndexDataPtr())) {
			ERROR_LOG("| VulkanMesh buffer creation failed:");
			ERROR_LOG("|-- Failed to create staging buffer.");
			return false;
		}

		// Create the index buffer.
		if (!device.CreateBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		                         m_Ibo,
		                         indexBufferSize,
		                         nullptr)) {
			ERROR_LOG("| VulkanMesh buffer creation failed:");
			ERROR_LOG("|-- Failed to create index buffer.");
			return false;
		}

		if (!device.CopyBuffer(stagingBuffer, m_Ibo, device.GetQueue(QueueFamily::TRANSFER))) {
			ERROR_LOG("| VulkanMesh buffer creation failed:");
			ERROR_LOG("|-- Failed to transfer data from the staging buffer to the index buffer");
			return false;
		}
	}

	//The staging buffer goes out of scope and cleans up itself.

	return true;
}

void VulkanMesh::Draw(VkCommandBuffer commandBuffer) noexcept
{
	//Bind the vbo.
	VkDeviceSize offsets{ 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_Vbo.buffer, &offsets);

	//if the mesh has indices.
	if (!GetIndices().empty()) {
		//Bind the ibo.
		vkCmdBindIndexBuffer(commandBuffer, m_Ibo.buffer, 0, VK_INDEX_TYPE_UINT32);

		// Record draw indexed command.
		vkCmdDrawIndexed(commandBuffer, static_cast<ui32>(GetIndices().size()), 1, 0, 0, 0);
	} else {
		//the mesh has no indices, record simple draw command.
		vkCmdDraw(commandBuffer, static_cast<ui32>(GetVertices().size()), 1, 0, 0);
	}
}

