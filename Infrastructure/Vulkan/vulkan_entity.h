#ifndef VULKAN_ENTITY_H_
#define VULKAN_ENTITY_H_
#include "entity.h"
#include "vulkan_mesh.h"
#include "vulkan_material.h"

class VulkanEntity : public Entity {
private:
	VulkanMesh* m_Mesh{ nullptr };

	VulkanMaterial* m_Material;

public:
	explicit VulkanEntity(VulkanMesh* mesh);

	VulkanMaterial& GetMaterial() const noexcept;

	void SetMaterial(VulkanMaterial* material) noexcept;

	void Draw(VkCommandBuffer commandBuffer) noexcept;
};

#endif //VULKAN_ENTITY_H_