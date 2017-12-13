#ifndef DEMO_ENTITY_H_
#define DEMO_ENTITY_H_

#include "entity.h"
#include "vulkan_mesh.h"
#include "demo_material.h"

class DemoEntity final : public Entity {
private:
	std::string m_Name;

	VulkanMesh* m_Mesh{ nullptr };

	DemoMaterial* m_Material{ nullptr };

public:
	DemoEntity() = default;

	explicit DemoEntity(VulkanMesh* mesh);

	DemoMaterial* GetMaterial() noexcept;

	void SetName(const std::string& name) noexcept;

	void SetMesh(VulkanMesh* mesh) noexcept;

	VulkanMesh* GetMesh() const noexcept;

	void SetMaterial(DemoMaterial* material) noexcept;

	bool Load(const std::string& fileName) noexcept override;
};

#endif //DEMO_ENTITY_H_
