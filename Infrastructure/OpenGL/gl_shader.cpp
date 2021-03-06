#include "gl_shader.h"
#include "logger.h"
#include <vector>
#include <fstream>
#include <array>


// Private methods --------------------------------------------
bool GLShader::CompileText(const std::string& fileName) const noexcept
{
    auto pos = fileName.rfind('.');
    const std::string trimmedFname{ fileName.substr(0, pos) };

    std::ifstream file{ trimmedFname };

    if (!file.is_open()) {
        ERROR_LOG("Failed to open file: " + fileName);
        return false;
    }

    const std::string contents{ std::istreambuf_iterator<char>(file),
                                std::istreambuf_iterator<char>() };

    assert(!contents.empty());

    const char* cptr = contents.c_str();
    glShaderSource(m_Id, 1, &cptr, nullptr);
    assert(glGetError() == GL_NO_ERROR);

    glCompileShader(m_Id);
    assert(glGetError() == GL_NO_ERROR);

    GLint status{ 0 };
    glGetShaderiv(m_Id, GL_COMPILE_STATUS, &status);
    assert(glGetError() == GL_NO_ERROR);

    if (status != GL_TRUE) {
        GLint messageSize{ 0 };
        glGetShaderiv(m_Id, GL_INFO_LOG_LENGTH, &messageSize);
        assert(glGetError() == GL_NO_ERROR);

        std::string infoMessage;
        infoMessage.resize(messageSize);

        glGetShaderInfoLog(m_Id, infoMessage.size(), nullptr, infoMessage.data());
        assert(glGetError() == GL_NO_ERROR);

        ERROR_LOG(infoMessage);

        return false;
    }


    return true;
}

bool GLShader::LoadSpirv(const std::string& fileName) const noexcept
{
    std::ifstream file{ fileName, std::ios::ate | std::ios::binary };

    if (!file.is_open()) {
        ERROR_LOG("Failed to open file: " + fileName);
        return false;
    }

    const auto fileSize = file.tellg();

    assert(fileSize > 0);

    std::vector<byte> buffer;
    buffer.resize(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    // Associate spir-v module with shader object
    glShaderBinary(1,
                   &m_Id,
                   GL_SHADER_BINARY_FORMAT_SPIR_V_ARB,
                   buffer.data(),
                   buffer.size());
    assert(glGetError() == GL_NO_ERROR);

    // Specify shader module entry point and specialization cosntants
    glSpecializeShaderARB(m_Id, //obj id
                          "main", // shader module entry point
                          0, // no specialization constants
                          nullptr, // no specialization constants
                          nullptr); // no specialization constants
    assert(glGetError() == GL_NO_ERROR);

    GLint status{ 0 };
    glGetShaderiv(m_Id, GL_COMPILE_STATUS, &status);
    assert(glGetError() == GL_NO_ERROR);

    if (status != GL_TRUE) {
        GLint messageSize{ 0 };
        glGetShaderiv(m_Id, GL_INFO_LOG_LENGTH, &messageSize);
        assert(glGetError() == GL_NO_ERROR);

        std::string infoMessage;
        infoMessage.resize(messageSize);

        glGetShaderInfoLog(m_Id, infoMessage.size(), nullptr, infoMessage.data());
        assert(glGetError() == GL_NO_ERROR);

        ERROR_LOG(infoMessage);

        return false;
    }


    return true;
}
// ------------------------------------------------------------

GLShader::GLShader(const GLShaderStageType type) noexcept
	: m_Type{ type }
{
    switch (m_Type) {
    case VERTEX:
        m_Id = glCreateShader(GL_VERTEX_SHADER);
        break;
    case TESSELATION_CONTROL:
        m_Id = glCreateShader(GL_TESS_CONTROL_SHADER);
        break;
    case TESSELATION_EVALUATION:
        m_Id = glCreateShader(GL_TESS_EVALUATION_SHADER);
        break;
    case GEOMETRY:
        m_Id = glCreateShader(GL_GEOMETRY_SHADER);
        break;
    case FRAGMENT:
        m_Id = glCreateShader(GL_FRAGMENT_SHADER);
        break;
    case COMPUTE:
        m_Id = glCreateShader(GL_COMPUTE_SHADER);
        break;
    default:
        ERROR_LOG("Unknown shader type! Aborting!");
    }
}

GLShader::~GLShader()
{
	glDeleteShader(m_Id);
}

bool GLShader::Load(const std::string& fileName) noexcept
{
    GLint numFormats{ 0 };
    glGetIntegerv(GL_NUM_SHADER_BINARY_FORMATS, &numFormats);

    auto spirv = false;

    if (numFormats) {
        std::vector<GLint> formats;
        formats.resize(numFormats);

        glGetIntegerv(GL_SHADER_BINARY_FORMATS, formats.data());

        for (const auto format : formats) {
            if (format & GL_SHADER_BINARY_FORMAT_SPIR_V_ARB) {
                spirv = true;
                break;
            }
        }
    }

//    return spirv ? LoadSpirv(fileName) : CompileText(fileName);
	return CompileText(fileName);
}

GLShaderStageType GLShader::GetType() const noexcept
{
	return m_Type;
}

GLuint GLShader::GetId() const noexcept
{
	return m_Id;
}

