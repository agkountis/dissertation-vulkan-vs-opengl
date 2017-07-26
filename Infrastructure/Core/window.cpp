#include "window.h"

const std::string& Window::GetTitle() const noexcept
{
	return m_Title;
}

void Window::SetTitle(const std::string title) noexcept
{
	m_Title = title;
}

const Vec2ui& Window::GetSize() const noexcept
{
	return m_Size;
}

void Window::SetSize(const Vec2ui& size) noexcept
{
	m_Size = size;
}

const Vec2i& Window::GetPosition() const noexcept
{
	return m_Position;
}

void Window::SetPosition(const Vec2i& position) noexcept
{
	m_Position = position;
}

const Vec2i& Window::GetCursorPosition() const noexcept
{
	return m_CursorPosition;
}

void Window::SetCursorPosition(const Vec2i& cursorPosition) noexcept
{
	m_CursorPosition = cursorPosition;
}
