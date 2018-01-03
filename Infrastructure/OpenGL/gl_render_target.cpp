#include "gl_render_target.h"
#include "logger.h"

static std::vector<GLuint> s_DepthFormats{
	GL_DEPTH_COMPONENT16,
	GL_DEPTH_COMPONENT24,
	GL_DEPTH_COMPONENT32,
	GL_DEPTH_COMPONENT32F
};

GLRenderTarget::~GLRenderTarget()
{
	glDeleteFramebuffers(1, &m_Id);
}

bool GLRenderTarget::Create(const std::vector<GLRenderTargetAttachmentCreateInfo>& attachmentsInfos) noexcept
{
	glCreateFramebuffers(1, &m_Id);

	if (!attachmentsInfos.empty()) {
		m_Attachments.resize(attachmentsInfos.size());

		glCreateTextures(GL_TEXTURE_2D, attachmentsInfos.size(), m_Attachments.data());

		auto colorAttachmentCount = 0;
		for (auto i = 0u; i < attachmentsInfos.size(); ++i) {
			glTextureStorage2D(m_Attachments[i],
			                   1,
			                   attachmentsInfos[i].internalFormat,
			                   attachmentsInfos[i].size.x,
			                   attachmentsInfos[i].size.y);

			auto isDepthAttachment = false;

			for (const auto depthFormat : s_DepthFormats) {
				if (attachmentsInfos[i].internalFormat == depthFormat) {
					isDepthAttachment = true;
					break;
				}
			}

			if (isDepthAttachment && !m_HasDepth) {
				glNamedFramebufferTexture(m_Id, GL_DEPTH_ATTACHMENT, m_Attachments[i], 0);
				m_HasDepth = true;
			}
			else {
				const auto outputLocation = GL_COLOR_ATTACHMENT0 + colorAttachmentCount++;
				glNamedFramebufferTexture(m_Id, outputLocation, m_Attachments[i], 0);
				glNamedFramebufferDrawBuffer(m_Id, outputLocation);
			}
		}
	}

	const auto status = glCheckNamedFramebufferStatus(m_Id, GL_DRAW_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		switch (status) {
		case GL_FRAMEBUFFER_UNDEFINED:
			ERROR_LOG("Undefined framebuffer");
			return false;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			ERROR_LOG("Incomplete framebuffer attachment.");
			return false;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			ERROR_LOG("Incomplete framebuffer. Add at least one attachment to the render target.");
			return false;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
			ERROR_LOG("Incomplete draw buffer. Check that all attachments enabled exist in the render target.");
			return false;
		default:
			ERROR_LOG("Render target error.");
			return false;
		}
	}

	return true;
}

void GLRenderTarget::Bind() const noexcept
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_Id);
}

void GLRenderTarget::Unbind() const noexcept
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
