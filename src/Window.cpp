#include "Window.h"
#include "Renderer/Deferred.h"

Window::Window()
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

	m_renderer = renderer::Deferred::create(m_window);
	m_renderer->init();
}

Window::~Window()
{
	glfwDestroyWindow(m_window);
	glfwTerminate();
}

void Window::onUpdate() noexcept
{
	glfwPollEvents();

	m_renderer->render();
}