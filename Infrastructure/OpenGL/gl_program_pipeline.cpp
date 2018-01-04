#include "gl_program_pipeline.h"

GLProgramPipeline::~GLProgramPipeline()
{
	glDeleteProgramPipelines(1, &m_Id);
}

void GLProgramPipeline::AddShader(const GLShader* shader) noexcept
{
	if (m_Shaders[shader->GetType()]) {
		ERROR_LOG("Shader of type " + GLShader::TypeToString(shader->GetType()) + " already exists in the pipeline.");
	}

	m_Shaders[shader->GetType()] = shader;
}

void GLProgramPipeline::SetRenderTarget(const GLRenderTarget* renderTarget) noexcept
{
	m_pRenderTarget = renderTarget;
}

bool GLProgramPipeline::Create() noexcept
{
	if (m_Shaders.empty()) {
		ERROR_LOG(
			"No shaders attached to the pipeline. Please use AddShader(...) to add shaders to the pipeline before calling Create()."
		);
		return false;
	}

	glCreateProgramPipelines(1, &m_Id);
	glBindProgramPipeline(m_Id);
	assert(glGetError() == GL_NO_ERROR);

	for (const auto shader : m_Shaders) {

		if (!shader) {
			continue;
		}

		const auto progId = glCreateProgram();
		assert(glGetError() == GL_NO_ERROR);

		if (!progId) {
			ERROR_LOG("Failed to create Shader program for shader: { id:" + std::to_string(shader->GetId()) + ", type:" +
				GLShader::TypeToString(shader->GetType()) + "}");
			return false;
		}

		glProgramParameteri(progId, GL_PROGRAM_SEPARABLE, GL_TRUE); //must be called before linking
		assert(glGetError() == GL_NO_ERROR);

		glAttachShader(progId, shader->GetId());
		assert(glGetError() == GL_NO_ERROR);

		glLinkProgram(progId);
		assert(glGetError() == GL_NO_ERROR);

		GLint linkStatus{ 0 };
		glGetProgramiv(progId, GL_LINK_STATUS, &linkStatus);
		assert(glGetError() == GL_NO_ERROR);

		if (linkStatus != GL_TRUE) {
			ERROR_LOG("Failed to link Shader program for shader: { id:" + std::to_string(shader->GetId()) + ", type:" + GLShader
				::TypeToString(shader->GetType()) + "}");

			GLint errBufferSize = 0;
			glGetProgramiv(progId, GL_INFO_LOG_LENGTH, &errBufferSize);
			assert(glGetError() == GL_NO_ERROR);

			std::string err;
			err.resize(errBufferSize);

			glGetProgramInfoLog(progId, errBufferSize, nullptr, err.data());
			assert(glGetError() == GL_NO_ERROR);

			ERROR_LOG(err);

			return false;
		}

		glDetachShader(progId, shader->GetId());
		assert(glGetError() == GL_NO_ERROR);

		m_ShaderPrograms[shader->GetType()] = progId;

		glUseProgramStages(m_Id, GLShader::GLType(shader->GetType()), progId);
		assert(glGetError() == GL_NO_ERROR);
	}

	assert(glGetError() == GL_NO_ERROR);

	return true;
}

void GLProgramPipeline::Bind() const noexcept
{
	if (!glIsProgramPipeline(m_Id)) {
		return;
	}
	glBindProgramPipeline(m_Id);

	if (m_pRenderTarget) {
		m_pRenderTarget->Bind();
	}
	else {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	assert(glGetError() == GL_NO_ERROR);
}

void GLProgramPipeline::Unbind() const noexcept
{
	glBindProgramPipeline(0);
}

void GLProgramPipeline::Clear() noexcept
{
	if (m_pRenderTarget) {
		for (auto i = 0u; i < m_pRenderTarget->GetAttachmentCount(); ++i) {
			glClearNamedFramebufferfv(m_pRenderTarget->GetId(), GL_COLOR, i, glm::value_ptr(m_ColorClearValue));
			assert(glGetError() == GL_NO_ERROR);
			glClearNamedFramebufferfv(m_pRenderTarget->GetId(), GL_DEPTH, 0, &m_DepthClearValue);
			assert(glGetError() == GL_NO_ERROR);
		}
	} else {
		glClearBufferfv(GL_COLOR, 0, glm::value_ptr(m_ColorClearValue));
		assert(glGetError() == GL_NO_ERROR);
		glClearBufferfv(GL_DEPTH, 0, &m_DepthClearValue);
		assert(glGetError() == GL_NO_ERROR);
	}
}

void GLProgramPipeline::SetColorClearValue(const Vec4f& colorClearValue) noexcept
{
	m_ColorClearValue = colorClearValue;
}

void GLProgramPipeline::SetDepthClearValue(const f32 depthClearValue) noexcept
{
	m_DepthClearValue = depthClearValue;
}

void GLProgramPipeline::SetMatrix4f(const std::string& name, const Mat4f& value, const GLShaderStageType stage)
{
	const auto programId = m_ShaderPrograms[stage];
	const auto location = glGetProgramResourceLocation(programId, GL_UNIFORM, name.c_str());

	if (location < 0) {
		ERROR_LOG("Uniform: " + name + " is not active or does not exists in shader with ID: " + std::to_string(programId));
		return;
	}

	glProgramUniformMatrix4fv(programId, location, 1, GL_FALSE, glm::value_ptr(value));
}

void GLProgramPipeline::SetTexture(const std::string& name,
                                   const GLTexture* texture,
                                   const GLTextureSampler& sampler,
                                   const GLShaderStageType stage)
{
	const auto programId = m_ShaderPrograms[stage];
	const auto location = glGetProgramResourceLocation(programId, GL_UNIFORM, name.c_str());

	if (location < 0) {
		ERROR_LOG("Uniform: " + name + " is not active or does not exist in shader with ID: " + std::to_string(programId));
		return;
	}

	glBindTextureUnit(location, texture->GetId());
	glBindSampler(location, sampler.GetId());
}

void GLProgramPipeline::SetTexture(const std::string& name,
                                   const GLuint textureId,
                                   const GLTextureSampler& sampler,
                                   const GLShaderStageType stage)
{
	const auto programId = m_ShaderPrograms[stage];
	const auto location = glGetProgramResourceLocation(programId, GL_UNIFORM, name.c_str());

	if (location < 0) {
		ERROR_LOG("Uniform: " + name + " is not active or does not exist in shader with ID: " + std::to_string(programId));
		return;
	}

	glBindTextureUnit(location, textureId);
	glBindSampler(location, sampler.GetId());
}

void GLProgramPipeline::SetInteger(const std::string& name, const GLint value, const GLShaderStageType stage)
{
	const auto programId = m_ShaderPrograms[stage];
	const auto location = glGetProgramResourceLocation(programId, GL_UNIFORM, name.c_str());

	if (location < 0) {
		ERROR_LOG("Uniform: " + name + " is not active or does not exist in shader with ID: " + std::to_string(programId));
		return;
	}

	glProgramUniform1i(programId, location, value);
}
