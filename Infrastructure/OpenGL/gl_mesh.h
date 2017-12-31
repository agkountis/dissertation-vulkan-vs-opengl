#ifndef GL_MESH_H_
#define GL_MESH_H_
#include "mesh.h"
#include <GL/glew.h>

class GLMesh final : public Mesh {
private:
	GLuint m_Vao{ 0 };

	GLuint m_Vbo{ 0 };

	GLuint m_Ibo{ 0 };

public:
	GLMesh() = default;

	~GLMesh();

	bool CreateBuffers() noexcept override;

	void Draw() const noexcept;
};

#endif //GL_MESH_H_
