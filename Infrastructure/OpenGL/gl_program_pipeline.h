#ifndef GL_PROGRAM_PIPELINE_H_
#define GL_PROGRAM_PIPELINE_H_
#include <GL/glew.h>
#include <array>
#include "gl_shader.h"
#include <string>
#include "gl_texture.h"
#include "gl_texture_sampler.h"
#include "gl_render_target.h"

class GLProgramPipeline final {
private:
	GLuint m_Id{ 0 };

	std::array<const GLShader*, 6> m_Shaders{};
	std::array<GLuint, 6> m_ShaderPrograms{};

	const GLRenderTarget* m_pRenderTarget{ nullptr };

public:
	~GLProgramPipeline();

	void AddShader(const GLShader* shader) noexcept;

	void SetRenderTarget(const GLRenderTarget* renderTarget) noexcept;

	bool Create() noexcept;

	void Bind() const noexcept;

	void Unbind() const noexcept;

	void SetMatrix4f(const std::string& name, const Mat4f& value, const GLShaderStageType stage);

	void SetTexture(const std::string& name, const GLTexture* texture, const GLTextureSampler& sampler, const GLShaderStageType stage);
};

#endif //GL_PROGRAM_PIPELINE_H_
