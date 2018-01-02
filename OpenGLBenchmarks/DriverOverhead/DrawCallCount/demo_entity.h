#ifndef DEMO_ENTITY_H_
#define DEMO_ENTITY_H_
#include "entity.h"
#include "demo_material.h"
#include "gl_mesh.h"

class DemoEntity final : public Entity {
private:
	GLMesh* m_Mesh{ nullptr };

	DemoMaterial* m_Material{ nullptr };

public:
	explicit DemoEntity(GLMesh* mesh);

	DemoMaterial& GetMaterial() const noexcept;

	void SetMaterial(DemoMaterial* material) noexcept;

	void Draw() const noexcept;

	bool Load(const std::string& fileName) noexcept override;
};

#endif //DEMO_ENTITY_H_
