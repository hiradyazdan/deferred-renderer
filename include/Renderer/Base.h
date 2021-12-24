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

			void setupCommands()								noexcept;
			void setupRenderPassCommands(
				const vk::RenderPass::Data::BeginInfo &_beginInfo,
				const VkCommandBuffer									&_cmdBuffer
			)	noexcept;

		private:
			static std::vector<const char*> getSurfaceExtensions() noexcept;

		protected:
			struct ScreenData
			{
				friend class Base;

				Model model;

				private:
					vk::RenderPass::Data					renderPassData;
					vk::Pipeline::Data						pipelineData; // composition
					std::unique_ptr<vk::Material>	material;
			};

			std::unique_ptr<vk::Device>	m_device	= nullptr;
			GLFWwindow *m_window									= nullptr;

		private:
			ScreenData m_screenData;
	};
}