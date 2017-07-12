#include "window.h"

Window::Window(const std::string& title,
               const Vec2ui& size,
               const Vec2i& position)
	: title{ title },
	  size{ size },
	  position{ position }
{
}
