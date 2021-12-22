#include "Window.h"
#include "Renderer.h"

Window::Window()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	m_window = glfwCreateWindow(m_data.width, m_data.height, m_data.title, nullptr, nullptr);

	m_renderer = Renderer::create(m_window);
	m_renderer->init(m_data);
	m_renderer->prepare();

	glfwSetWindowUserPointer(m_window, &m_data);
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