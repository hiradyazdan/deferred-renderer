#pragma once

#include "Window.h"

class App
{
	public:
		App() : m_window(new Window()) {}

	public:
		void run() noexcept;

	private:
		Window *m_window = nullptr;
};