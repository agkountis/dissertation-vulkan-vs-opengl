#ifndef GL_RENDER_TARGET_H_
#define GL_RENDER_TARGET_H_
#include "GL/glew.h"
#include <vector>
#include "types.h"

struct GLRenderTargetAttachmentCreateInfo final {
	Vec2ui size;
	GLuint internalFormat{ 0 };
};

class GLRenderTarget final {
private:
	GLuint m_Id{ 0 };

	std::vector<GLuint> m_Attachments;

	bool m_HasDepth{ false };

public:
	~GLRenderTarget();

	bool Create(const std::vector<GLRenderTargetAttachmentCreateInfo>& attachmentsInfos) noexcept;

	void Bind() const noexcept;

	void Unbind() const noexcept;

	GLuint GetAttachment(const i32 index) const noexcept;

	size_t GetAttachmentCount() const noexcept;

	GLuint GetId() const noexcept;
};

#endif //GL_RENDER_TARGET_H_
