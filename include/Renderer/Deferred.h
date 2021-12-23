#pragma once

#include "Base.h"

namespace renderer
{
	class Deferred : public Base
	{
		public:
			inline static Deferred *create(GLFWwindow *_window) noexcept
			{
				return s_instance == nullptr
							 ? new Deferred(_window)
							 : s_instance;
			}

		public:
			void init(Window::WindowData &_winData) noexcept override;

		private:
			explicit Deferred(GLFWwindow *_window) : Base(_window) {}

		private:
			void initCmdBuffer()				noexcept;
			void initSyncObject()				noexcept;

		private:
			void submitOffscreenQueue() noexcept;
			void submitSceneQueue()			noexcept override;
			void draw()									noexcept override;

		private:
			inline static Deferred *s_instance = nullptr;
	};
}