#pragma once

#include "../Window.h"
#include "../vk.h"

#include "../Model.h"

namespace renderer
{
	class Base
	{
		public:
			~Base() = default;

		public:
			virtual void init(Window::WindowData &_winData)	noexcept;
			virtual void render() noexcept;

		protected:
			explicit Base(GLFWwindow *_window)
			: m_window(_window)
			, m_device(std::make_unique<vk::Device>()) {}

		protected:
			void loadAssets()										noexcept;

			void beginFrame()										noexcept;
			void endFrame()											noexcept;

			virtual void draw()									noexcept;
			virtual void submitSceneQueue()			noexcept;

		private:
			void initSwapchain()								noexcept;
			void initSyncObjects()							noexcept;
			void initGraphicsQueueSubmitInfo()	noexcept;
			void initCommands()									noexcept;

		private:
			static std::vector<const char*> getSurfaceExtensions() noexcept;

		protected:
			std::unique_ptr<vk::Device>	m_device	= nullptr;
			GLFWwindow *m_window									= nullptr;

			Model	m_model;
	};
}