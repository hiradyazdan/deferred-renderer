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
			virtual void init()	noexcept;
			virtual void render() noexcept {}

		protected:
			explicit Base(GLFWwindow *_window)
			: m_window(_window)
			, m_device(std::make_unique<vk::Device>()) {}

		protected:
			void loadAssets()								noexcept;

			void beginFrame()								noexcept;
			void endFrame()									noexcept;

			void setupCommands(
				const VkPipeline				&_pipeline,
				const VkPipelineLayout	&_pipelineLayout,
				const VkDescriptorSet		*_descSets,
				uint32_t								_descSetCount = 1
			) noexcept;

			inline VkRenderPass &getScreenRenderPass() noexcept
			{ return m_screenData.m_renderPassData.framebufferData.renderPass; }

		private:
			void initVk()												noexcept;
			void initCamera()										noexcept;
			void initCommands()									noexcept;
			void initSyncObjects()							noexcept;
			void initGraphicsQueueSubmitInfo()	noexcept;

			void setupRenderPass()							noexcept;
			void setupFramebuffers()						noexcept;
			void resizeWindow()									noexcept;

			void setupRenderPassCommands(
				const VkCommandBuffer		&_cmdBuffer,
				const VkPipeline				&_pipeline,
				const VkPipelineLayout	&_pipelineLayout,
				const VkDescriptorSet		*_descSets,
				uint32_t								_descSetCount = 1
			)	noexcept;

		private:
			virtual void setupCommands()		noexcept = 0;
			virtual void draw()							noexcept;
			virtual void submitSceneQueue()	noexcept;

		private:
			static void framebufferResize(GLFWwindow *_window, int _width, int _height) noexcept;
			static std::vector<const char*> getSurfaceExtensions() noexcept;

		protected:
			struct ScreenData
			{
				friend class Base;

				Camera													camera;
				Model														model;

				bool														isInited 	= false;
				bool														isPaused	= false;
				bool														isResized	= false;

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
					VkPipelineStageFlags					m_submitPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

					RenderPassData								m_renderPassData;
					std::unique_ptr<vk::Material>	m_material;
			};

			std::unique_ptr<vk::Device>	m_device		= nullptr;
			GLFWwindow									*m_window		= nullptr;

		protected:
			ScreenData m_screenData;
	};
}