#pragma once

#include <GLFW/glfw3.h>

#include "_constants.h"

namespace renderer
{
	class Base;
	class Deferred;
}

class Window
{
	friend class renderer::Base;
	friend class renderer::Deferred;

	public:
		struct WindowResize
		{
			static void resize(GLFWwindow *_window, int _width, int _height) noexcept
			{
				auto window = reinterpret_cast<WindowResize*>(glfwGetWindowUserPointer(_window));

				glfwGetFramebufferSize(_window, &_width, &_height);
			}
		};

	public:
		Window();
		~Window();

	public:
		GLFWwindow *getWindow() { return m_window; }

		void onUpdate() noexcept;

	private:
		GLFWwindow					*m_window;
		renderer::Deferred	*m_renderer;

		struct WindowData
		{
			const char* title	= constants::WINDOW_TITLE;
			int width					= constants::WINDOW_WIDTH;
			int height				= constants::WINDOW_HEIGHT;
		} m_data;
};