#pragma once

#include "vk/vk.h"

#include "../Camera.h"
#include "../AssetHelper.h"

class GLFWwindow;

inline const uint16_t vk::Attachment::s_attCount			= 2;
inline const uint16_t vk::RenderPass::s_subpassCount	= 1;
inline const uint16_t vk::RenderPass::s_spDepCount		= 2;

namespace renderer
{
	class Base
	{
		public:
			template<typename TRenderer>
			inline static TRenderer *create(GLFWwindow *_window) noexcept
			{
				s_instance = s_instance == nullptr
										 ? new TRenderer(_window)
										 : s_instance;

				return static_cast<TRenderer*>(s_instance);
			}

		public:
			virtual ~Base() = default; // TODO: clean up vk resources

		public:
			virtual void init()				noexcept;
			virtual void render()			noexcept = 0;
			virtual void loadAssets()	noexcept = 0;

		protected:
			explicit Base(GLFWwindow *_window)
			: m_window(_window)
			, m_device(std::make_unique<vk::Device>()) {}

		protected:
			void beginFrame()								noexcept;
			void endFrame()									noexcept;

			void setupCommands(
				const VkPipeline				&_pipeline,
				const VkPipelineLayout	&_pipelineLayout,
				const VkDescriptorSet		*_descSets,
				uint32_t								_descSetCount = 1
			) noexcept;

			template<
			  vk::Model::ID							modelId,
				vk::Model::RenderingMode	renderMode,
				vk::Buffer::Type					type,
				uint16_t									bufferCount
			>
			void loadAsset(
				vk::Buffer::Data<type, bufferCount>	&_bufferData,
				float 															_scale					= 1.0f,
				uint32_t														_instanceCount	= 1,
				uint32_t														_firstInstance	= 0,
				uint32_t														_firstIndex			= 0,
				int																	_vtxOffset			= 0
			) noexcept
			{
				auto &model = m_screenData.modelsData[modelId];

				AssetHelper::load(
					constants::MODELS_PATH + constants::models[modelId],
					model, _scale
				);
				vk::Model::setup<renderMode>(
					m_device, model, _bufferData,
					_instanceCount, _firstInstance,
					_firstIndex, _vtxOffset
				);
			}

			inline auto &getFbData() noexcept
			{ return m_screenData.m_renderPassData.framebufferData; }

			inline auto &getScreenRenderPass() noexcept
			{ return getFbData().renderPass; }

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
				vk::Vector<vk::Model::Data>			modelsData;

				bool														isInited 	= false;
				bool														isPaused	= false;
				bool														isResized	= false;

				private:
					using RenderPassData = vk::RenderPass::Data<
						vk::Attachment::s_attCount,
						vk::RenderPass::s_subpassCount,
						vk::RenderPass::s_spDepCount
					>;

					RenderPassData								m_renderPassData;
			};

			std::unique_ptr<vk::Device>	m_device		= nullptr;
			GLFWwindow									*m_window		= nullptr;

		protected:
			ScreenData m_screenData;

		private:
			inline static Base *s_instance = nullptr;
	};
}