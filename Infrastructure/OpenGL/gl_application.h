#ifndef GL_APPLICATION_H_
#define GL_APPLICATION_H_
#include "application.h"
#include "gl_window.h"
#include "resource_manager.h"

class GLApplication : public Application {
private:
	GLWindow m_Window;

	ResourceManager m_ResourceManager;

public:
	explicit GLApplication(const ApplicationSettings& settings) noexcept;

	bool Initialize() noexcept override;

	i32 Run() noexcept override;

	void PreDraw() noexcept override;

	void PostDraw() noexcept override;

	GLWindow& GetWindow() noexcept;

	void Reshape(const Vec2ui& size) noexcept;
};

#endif //GL_APPLICATION_H_