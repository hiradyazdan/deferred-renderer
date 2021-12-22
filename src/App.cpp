#include "App.h"

void App::run() noexcept
{
	while(
		m_window != nullptr &&
		!glfwWindowShouldClose(m_window->getWindow())
	)
	{
		m_window->onUpdate();
	}
}