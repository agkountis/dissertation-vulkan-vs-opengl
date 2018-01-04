#ifndef DEMO_ENTITY_H_
#define DEMO_ENTITY_H_
#include "entity.h"
#include "demo_material.h"
#include "gl_mesh.h"
#include <string>

class DemoEntity final : public Entity {
private:
	std::string m_Name;

	GLMesh* m_Mesh{ nullptr };

	DemoMaterial* m_Material{ nullptr };

public:
	DemoEntity() = default;

	explicit DemoEntity(GLMesh* mesh);

	void SetName(const std::string& name) noexcept;

	DemoMaterial* GetMaterial() const noexcept;

	void SetMaterial(DemoMaterial* material) noexcept;

	void SetMesh(GLMesh* mesh) noexcept;

	GLMesh* GetMesh() const noexcept;

	void Draw() const noexcept;

	bool Load(const std::string& fileName) noexcept override;
};

#endif //DEMO_ENTITY_H_