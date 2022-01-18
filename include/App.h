#pragma once

#include "Window.h"

template<typename TRenderer>
class App
{
	public:
		App() : m_window(new Window<TRenderer>()) {}
		App(const App&) = delete;
		App &operator=(const App&) = delete;
		~App() { delete m_window; }

	public:
		void run() noexcept
		{
			if(m_window == nullptr) return;

			while(!glfwWindowShouldClose(m_window->getWindow()))
			{
				m_window->onUpdate();
			}
		}

	private:
		Window<TRenderer> *m_window = nullptr;
};