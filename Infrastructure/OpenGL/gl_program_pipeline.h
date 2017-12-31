#ifndef GL_PROGRAM_PIPELINE_H_
#define GL_PROGRAM_PIPELINE_H_
#include <GL/glew.h>
#include <vector>
#include "gl_shader.h"

class GLProgramPipeline final {
private:
	GLuint m_Id{ 0 };

	std::vector<const GLShader*> m_Shaders;
	std::vector<GLuint> m_ShaderPrograms;

public:
	~GLProgramPipeline();

	void AddShader(const GLShader* shader) noexcept;

	bool Create() noexcept;

	void Bind() const noexcept;

	void Unbind() const noexcept;
};

#endif //GL_PROGRAM_PIPELINE_H_
