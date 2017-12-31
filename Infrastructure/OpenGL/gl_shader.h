#ifndef GL_SHADER_H_
#define GL_SHADER_H_
#include "resource.h"
#include <GL/glew.h>

enum GLShaderStageType {
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

	GLShaderStageType m_Type;

public:
	explicit GLShader(const GLShaderStageType type) noexcept;

	~GLShader();

	bool Load(const std::string& fileName) noexcept override;

	GLShaderStageType GetType() const noexcept;

	GLuint GetId() const noexcept;

	static std::string TypeToString(GLShaderStageType type)
	{
		switch (type) {
		case VERTEX:
			return "VERTEX";
		case TESSELATION_CONTROL:
			return "TESSELATION_CONTROL";
		case TESSELATION_EVALUATION:
			return "TESSELATION_EVALUATION";
		case GEOMETRY:
			return "GEOMETRY";
		case FRAGMENT:
			return "FRAGMENT";
		case COMPUTE:
			return "COMPUTE";
		default:
			return "VALUE NOT RECOGNIZED";
		}
	}

	static GLbitfield GLType(const GLShaderStageType type)
	{
		switch (type) {
		case VERTEX:
			return GL_VERTEX_SHADER_BIT;
		case TESSELATION_CONTROL:
			return GL_TESS_CONTROL_SHADER_BIT;
		case TESSELATION_EVALUATION:
			return GL_TESS_EVALUATION_SHADER_BIT;
		case GEOMETRY:
			return GL_GEOMETRY_SHADER_BIT;
		case FRAGMENT:
			return GL_FRAGMENT_SHADER_BIT;
		case COMPUTE:
			return GL_COMPUTE_SHADER_BIT;
		}

		return std::numeric_limits<unsigned int>::max();
	}
};

#endif //GL_SHADER_H_
