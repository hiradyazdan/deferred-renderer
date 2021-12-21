#pragma once

#include "Window.h"
#include "vk.h"

class Renderer
{
	public:
		inline static Renderer *create(GLFWwindow *_window)
		{
			return new Renderer(_window);
		}

	public:
		~Renderer() = default;

	public:
		void init(Window::WindowData &_winData)	noexcept;
		void initSwapchain()										noexcept;

	private:
		explicit Renderer(GLFWwindow *_window)
		: m_window(_window)
		, m_device(std::make_unique<vk::Device>()) {}

	private:
		static std::vector<const char*> getSurfaceExtensions() noexcept
		{
			uint32_t glfwExtensionCount = 0;
			auto glfwExtensions = glfwGetRequiredInstanceExtensions(
				&glfwExtensionCount
			);

			return
			{
				glfwExtensions,
				glfwExtensions + glfwExtensionCount
			};
		}

	private:
		GLFWwindow *m_window = nullptr;
		std::unique_ptr<vk::Device>	m_device = nullptr;
};