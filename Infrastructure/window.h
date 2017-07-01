#ifndef WINDOW_H_
#define WINDOW_H_
#include <string>
#include "types.h"

class Window {
private:
	std::string m_Title;

	Vec2i m_Size;

	Vec2i m_Position;

	Vec2i m_CursorPosition;

public:
	explicit Window(const std::string& title, const Vec2i& size, const Vec2i& position);

	Window(const Window& other) = delete;

	Window& operator=(const Window& other) = delete;

	Window(Window&& other) = delete;

	Window&& operator=(Window&& other) = delete;

	virtual ~Window() = default;

	virtual i32 MainLoop() noexcept = 0;
};

#endif //WINDOW_H_
