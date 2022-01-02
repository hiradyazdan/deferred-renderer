#pragma once

#include "../Window.h"
#include "../vk.h"

#include "../Camera.h"
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

			void setupCommands(
				const VkPipeline				&_pipeline,
				const VkPipelineLayout	&_pipelineLayout,
				const VkDescriptorSet		*_descSets,
				uint32_t								_descSetCount = 1
			) noexcept;
			static void setupRenderPassCommands(
				const VkExtent2D				&_swapchainExtent,
				const VkCommandBuffer		&_cmdBuffer,
				const VkPipeline				&_pipeline,
				const VkPipelineLayout	&_pipelineLayout,
				const VkDescriptorSet		*_descSets,
				uint32_t								_descSetCount = 1
			)	noexcept;

			inline VkRenderPass &getScreenRenderPass() noexcept
			{ return m_screenData.renderPassData.framebufferData.renderPass; }

		private:
			void initSyncObjects()							noexcept;
			void initGraphicsQueueSubmitInfo()	noexcept;
			void initCommands()									noexcept;

			void setupRenderPass()							noexcept;
			void setupFramebuffers()						noexcept;

		private:
			static std::vector<const char*> getSurfaceExtensions() noexcept;

		protected:
			struct ScreenData
			{
				friend class Base;

				Camera													camera;
				Model														model;

				private:
					inline static const uint16_t 	s_fbAttCount		= 2;
					inline static const uint16_t 	s_subpassCount	= 1;
					inline static const uint16_t 	s_spDepCount		= 2;
					using RenderPassData = vk::RenderPass::Data<
						s_fbAttCount,
						s_subpassCount,
						s_spDepCount
					>;

					// TODO
					VkPipelineStageFlags					submitPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

					RenderPassData								renderPassData;
					std::unique_ptr<vk::Material>	material;
			};

			std::unique_ptr<vk::Device>	m_device	= nullptr;
			GLFWwindow *m_window									= nullptr;

		private:
			ScreenData m_screenData;
	};
}