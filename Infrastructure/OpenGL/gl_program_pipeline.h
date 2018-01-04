#ifndef GL_PROGRAM_PIPELINE_H_
#define GL_PROGRAM_PIPELINE_H_
#include <GL/glew.h>
#include <array>
#include "gl_shader.h"
#include <string>
#include "gl_texture.h"
#include "gl_texture_sampler.h"
#include "gl_render_target.h"
#include "gl_buffer.h"
#include "logger.h"

class GLProgramPipeline final {
private:
	GLuint m_Id{ 0 };

	std::array<const GLShader*, 6> m_Shaders{};
	std::array<GLuint, 6> m_ShaderPrograms{};

	const GLRenderTarget* m_pRenderTarget{ nullptr };

	Vec4f m_ColorClearValue;
	f32 m_DepthClearValue{ 1.0 };

public:
	~GLProgramPipeline();

	void AddShader(const GLShader* shader) noexcept;

	void SetRenderTarget(const GLRenderTarget* renderTarget) noexcept;

	bool Create() noexcept;

	void Bind() const noexcept;

	void Unbind() const noexcept;

	void Clear() noexcept;

	void SetColorClearValue(const Vec4f& colorClearValue) noexcept;

	void SetDepthClearValue(const f32 depthClearValue) noexcept;

	void SetMatrix4f(const std::string& name, const Mat4f& value, const GLShaderStageType stage);

	void SetTexture(const std::string& name,
	                const GLTexture* texture,
	                const GLTextureSampler& sampler,
	                const GLShaderStageType stage);

	void SetTexture(const std::string& name,
	                const GLuint textureId,
	                const GLTextureSampler& sampler,
	                const GLShaderStageType stage);

	void SetInteger(const std::string& name, const GLint value, const GLShaderStageType stage);

	template <typename T>
	void SetUniformBuffer(const std::string& name, const GLBuffer<T>& buffer, const GLShaderStageType stage)
	{
		const auto programId = m_ShaderPrograms[stage];
		const auto index = glGetUniformBlockIndex(programId, name.c_str());

		if (index == GL_INVALID_INDEX) {
			ERROR_LOG("Cause: Invalid index -- Uniform block: " + name + "is not active or does not exist in shader with ID: " +
				std::to_string(programId));
			return;
		}

		GLint binding;
		glGetActiveUniformBlockiv(programId, index, GL_UNIFORM_BLOCK_BINDING, &binding);

		if (binding < 0) {
			ERROR_LOG("Uniform block: " + name + "is not active or does not exist in shader with ID: " + std::to_string(programId
			));
			return;
		}

		glBindBufferBase(GL_UNIFORM_BUFFER, binding, buffer.GetId());
	}
};

#endif //GL_PROGRAM_PIPELINE_H_
