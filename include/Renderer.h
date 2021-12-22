#pragma once

#include "Window.h"
#include "vk.h"

#include "Model.h"

class Renderer
{
	public:
		inline static Renderer *create(GLFWwindow *_window) noexcept
		{
			return s_instance == nullptr
			? new Renderer(_window)
			: s_instance;
		}

	public:
		~Renderer() = default;

	public:
		void init(Window::WindowData &_winData)	noexcept;

	public:
		void prepare() noexcept;
		void render() noexcept;

	private:
		explicit Renderer(GLFWwindow *_window)
		: m_window(_window)
		, m_device(std::make_unique<vk::Device>()) {}

	private:
		void initSwapchain()								noexcept;
		void initSyncObjects()							noexcept;
		void initGraphicsQueueSubmitInfo()	noexcept;

	private:
		static std::vector<const char*> getSurfaceExtensions() noexcept;

	private:
		void draw()									noexcept;
		void beginFrame()						noexcept;
		void endFrame()							noexcept;

	private:
		void loadAssets()						noexcept;

		void submitOffscreenQueue() noexcept;
		void submitSceneQueue()			noexcept;

	private:
		inline static Renderer *s_instance = nullptr;

		GLFWwindow *m_window = nullptr;
		std::unique_ptr<vk::Device>	m_device = nullptr;

		Model	m_model;
};