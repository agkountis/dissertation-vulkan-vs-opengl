#include "window.h"


Window::Window(const std::string& title,
               const Vec2i& size,
               const Vec2i& position)
	: m_Title{ title },
	  m_Size{ size },
	  m_Position{ position }
{
}
