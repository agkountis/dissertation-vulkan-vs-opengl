#ifndef WINDOW_H_
#define WINDOW_H_
#include <string>
#include "types.h"

struct Window {
	std::string title;

	Vec2ui size;

	Vec2i position;

	Vec2i cursorPosition;

	explicit Window(const std::string& title, const Vec2ui& size, const Vec2i& position);

	Window(const Window& other) = delete;

	Window& operator=(const Window& other) = delete;

	Window(Window&& other) = delete;

	Window&& operator=(Window&& other) = delete;

	virtual ~Window() = default;
};

#endif //WINDOW_H_
