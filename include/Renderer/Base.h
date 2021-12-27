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
			void initSyncObjects()							noexcept;
			void initGraphicsQueueSubmitInfo()	noexcept;
			void initCommands()									noexcept;

			void setupRenderPass()							noexcept;
			void setupFramebuffer()							noexcept;
			void setupPipeline()								noexcept;

			void setupCommands()								noexcept;
			void setupRenderPassCommands(
				const VkExtent2D			&_swapchainExtent,
				const VkCommandBuffer	&_cmdBuffer
			)	noexcept;

		private:
			static std::vector<const char*> getSurfaceExtensions() noexcept;

		protected:
			struct ScreenData
			{
				friend class Base;

				Model model;

				private:
					inline static const uint16_t s_fbAttCount		= 2;
					inline static const uint16_t s_spDescCount	= 1;
					inline static const uint16_t s_spDepCount		= 2;
					using RenderPassData = vk::RenderPass::Data<
						s_fbAttCount,
						s_spDescCount,
						s_spDepCount
					>;

					RenderPassData								renderPassData;
					vk::Pipeline::Data						pipelineData; // composition
					std::unique_ptr<vk::Material>	material;
			};

			std::unique_ptr<vk::Device>	m_device	= nullptr;
			GLFWwindow *m_window									= nullptr;

		private:
			ScreenData m_screenData;
	};
}