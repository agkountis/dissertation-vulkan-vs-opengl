#ifndef GL_SHADER_H_
#define GL_SHADER_H_
#include "resource.h"
#include <GL/glew.h>

enum class GLShaderType {
	VERTEX,
	TESSELATION_CONTROL,
	TESSELATION_EVALUATION,
	GEOMETRY,
	FRAGMENT,
	COMPUTE
};

class GLShader final : public Resource {
private:
	GLuint m_Id{ 0 };

	GLShaderType m_Type;

public:
	explicit GLShader(const GLShaderType type) noexcept;

	~GLShader();

	bool Load(const std::string& fileName) noexcept override;

	GLShaderType GetType() const noexcept;

	operator GLuint() const noexcept;

	static std::string TypeToString(GLShaderType type)
	{
		switch (type) {
		case GLShaderType::VERTEX:
			return "VERTEX";
		case GLShaderType::TESSELATION_CONTROL:
			return "TESSELATION_CONTROL";
		case GLShaderType::TESSELATION_EVALUATION:
			return "TESSELATION_EVALUATION";
		case GLShaderType::GEOMETRY:
			return "GEOMETRY";
		case GLShaderType::FRAGMENT:
			return "FRAGMENT";
		case GLShaderType::COMPUTE:
			return "COMPUTE";
		default:
			return "VALUE NOT RECOGNIZED";
		}
	}

	static GLbitfield GLType(const GLShaderType type)
	{
		switch (type) {
		case GLShaderType::VERTEX:
			return GL_VERTEX_SHADER_BIT;
		case GLShaderType::TESSELATION_CONTROL:
			return GL_TESS_CONTROL_SHADER_BIT;
		case GLShaderType::TESSELATION_EVALUATION:
			return GL_TESS_EVALUATION_SHADER_BIT;
		case GLShaderType::GEOMETRY:
			return GL_GEOMETRY_SHADER_BIT;
		case GLShaderType::FRAGMENT:
			return GL_FRAGMENT_SHADER_BIT;
		case GLShaderType::COMPUTE:
			return GL_COMPUTE_SHADER_BIT;
		}

		return std::numeric_limits<unsigned int>::max();
	}
};

#endif //GL_SHADER_H_
