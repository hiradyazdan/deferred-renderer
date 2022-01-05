#include "App.h"

void App::run() noexcept
{
	if(m_window == nullptr) return;

	while(!glfwWindowShouldClose(m_window->getWindow()))
	{
		m_window->onUpdate();
	}
}