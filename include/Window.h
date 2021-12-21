#pragma once

#include <GLFW/glfw3.h>

class Renderer;

class Window
{
	friend class Renderer;

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
		GLFWwindow *m_window;
		Renderer *m_renderer;
		struct WindowData
		{
			const char* title	= "Deferred Renderer";
			int width					= 1280;
			int height				= 720;
		} m_data;
};