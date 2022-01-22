#pragma once

#include <GLFW/glfw3.h>

#include "Renderer.h"
#include "EventHandler.h"

template<typename TRenderer>
class Window
{
	using Renderer			= renderer::Base;
	using EventHanlder	= EventHandler<TRenderer>;

	public:
		Window()
		{
			glfwInit();
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

			m_window = glfwCreateWindow(
				constants::WINDOW_WIDTH,
				constants::WINDOW_HEIGHT,
				constants::WINDOW_TITLE,
				nullptr, nullptr
			);

			m_renderer = Renderer::create<TRenderer>(m_window);
			m_renderer->init();

			m_eventHandler = EventHanlder::getInstance(m_window);
			m_eventHandler->setEventCallbacks();
		}
		~Window()
		{
			delete m_renderer;
			delete m_eventHandler;

			glfwDestroyWindow(m_window);
			glfwTerminate();
		}

	public:
		GLFWwindow *getWindow() noexcept { return m_window; }

		void onUpdate() noexcept
		{
			glfwPollEvents();

			m_renderer->render();
		}

	private:
		GLFWwindow		*m_window;
		TRenderer			*m_renderer;
		EventHanlder	*m_eventHandler;
};