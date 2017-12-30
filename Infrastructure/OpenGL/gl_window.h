#ifndef GL_WINDOW_H_
#define GL_WINDOW_H_
#include "window.h"
#include "GLFW/glfw3.h"

class GLWindow final : public Window {
private:
	GLFWwindow* m_Handle{ nullptr };

	static void OnWindowResize(GLFWwindow* window, i32 width, i32 height) noexcept;

public:
	~GLWindow();

	bool Create(const std::string& title,
	            const Vec2ui& size,
	            const Vec2i& position,
	            Application* application) noexcept override;

	operator GLFWwindow*() const;
};

#endif //GL_WINDOW_H_
