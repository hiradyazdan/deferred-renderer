#pragma once

#include <GLFW/glfw3.h>

#include "macros.h"

namespace renderer
{
	class Deferred;
}

class Window
{
	public:
		Window();
		~Window();

	public:
		GLFWwindow *getWindow() noexcept { return m_window; }

		void onUpdate() noexcept;

	private:
		GLFWwindow					*m_window;
		renderer::Deferred	*m_renderer;
};