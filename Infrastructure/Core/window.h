#ifndef WINDOW_H_
#define WINDOW_H_
#include <string>
#include "types.h"

class Application;

class Window {
private:
	std::string m_Title;

	Vec2ui m_Size;

	Vec2i m_Position;

	Vec2i m_CursorPosition;

public:
	Window() = default;

	Window(const Window& other) = delete;

	Window& operator=(const Window& other) = delete;

	Window(Window&& other) = delete;

	Window&& operator=(Window&& other) = delete;

	virtual ~Window() = default;

	virtual bool Create(const std::string& title,
	                    const Vec2ui& size,
	                    const Vec2i& position,
	                    Application* application) noexcept = 0;

	const std::string& GetTitle() const noexcept;

	void SetTitle(const std::string title) noexcept;

	const Vec2ui& GetSize() const noexcept;

	void SetSize(const Vec2ui& size) noexcept;

	const Vec2i& GetPosition() const noexcept;

	void SetPosition(const Vec2i& position) noexcept;

	const Vec2i& GetCursorPosition() const noexcept;

	void SetCursorPosition(const Vec2i& cursorPosition) noexcept;
};

#endif //WINDOW_H_
