#ifndef GL_MESH_H_
#define GL_MESH_H_
#include "mesh.h"
#include <GL/glew.h>

class GLMesh final : public Mesh {
private:
	GLuint m_Vao{ 0 };

	GLuint m_Vbo{ 0 };

	GLuint m_Ibo{ 0 };

	ui32 m_MaterialIndex{ 0 };

public:
	GLMesh() = default;

	~GLMesh();

	bool CreateBuffers() noexcept override;

	void Draw() const noexcept;

	void SetMaterialIndex(const ui32 materialIndex) noexcept
	{
		m_MaterialIndex = materialIndex;
	}

	ui32 GetMaterialIndex() const noexcept
	{
		return m_MaterialIndex;
	}
};

#endif //GL_MESH_H_
