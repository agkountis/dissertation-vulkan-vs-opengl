#ifndef DEMO_ENTITY_H_
#define DEMO_ENTITY_H_

#include "entity.h"
#include "vulkan_mesh.h"
#include "demo_material.h"

class DemoEntity : public Entity {
private:
	VulkanMesh* m_Mesh{ nullptr };

	DemoMaterial m_Material;

public:
	explicit DemoEntity(VulkanMesh* mesh);

	DemoMaterial& GetMaterial() noexcept;

	void SetMaterial(const DemoMaterial& material) noexcept;

	void Draw(VkCommandBuffer commandBuffer) noexcept;

	bool Load(const std::string& fileName) noexcept override;
};

#endif //DEMO_ENTITY_H_
