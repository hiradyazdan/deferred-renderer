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

			void setupCommands()				noexcept;
			void setupRenderPassCommands(
				const VkExtent2D			&_swapchainExtent,
				const VkCommandBuffer	&_cmdBuffer
			)	noexcept;

		private:
			void submitOffscreenQueue() noexcept;
			void submitSceneQueue()			noexcept override;
			void draw()									noexcept override;

		private:
			inline static Deferred *s_instance = nullptr;

			struct OffScreenData : ScreenData
			{
				inline static const uint16_t s_fbAttCount		= 4;
				inline static const uint16_t s_spDescCount	= 1;
				inline static const uint16_t s_spDepCount		= 2;
				using RenderPassData = vk::RenderPass::Data<
					s_fbAttCount,
					s_spDescCount,
					s_spDepCount
				>;

				RenderPassData	renderPassData;

				VkCommandBuffer	cmdBuffer	= VK_NULL_HANDLE;
				VkSemaphore			semaphore	= VK_NULL_HANDLE;
			} m_offscreenData;
	};
}