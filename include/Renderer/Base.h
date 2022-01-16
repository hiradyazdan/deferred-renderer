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
			  vk::Model::ID			modelId,	vk::Model::RenderingMode	renderMode = vk::Model::RenderingMode::PER_PRIMITIVE,
				vk::Buffer::Type	type, 		uint16_t									bufferCount
			>
			void loadAsset(
				vk::Buffer::Data<type, bufferCount>	&_bufferData,
				uint32_t														_instanceCount	= 1,
				uint32_t														_firstInstance	= 0,
				uint32_t														_firstIndex			= 0,
				int																	_vtxOffset			= 0,
				float 															_scale					= 1.0f
			) noexcept
			{
				auto &model = m_screenData.modelsData[modelId];
				auto &modelFile = constants::models[modelId];
				auto modelTexDir = std::string(modelFile).substr(0, std::string(modelFile).find('.'));

				AssetHelper::load(
					m_device,
					constants::MODELS_PATH + modelFile,
					model, _scale, constants::TEXTURES_PATH + modelTexDir + "/"
				);
				vk::Model::setup<renderMode>(
					m_device, model, _bufferData,
					{ _firstIndex, _vtxOffset, _instanceCount, _firstInstance }
				);
			}

			template<typename TScreenData>
			inline static auto &getFbData(TScreenData &_screenData) noexcept
			{
				static_assert(
					std::is_convertible<TScreenData, ScreenData>::value,
					"TScreenData should be of type ScreenData"
				);

				return _screenData.framebufferData;
			}

			template<typename TScreenData>
			inline static auto getScreenRenderPass(const TScreenData &_screenData) noexcept
			{ return getFbData(_screenData).renderPass; }

		private:
			void initVk()												noexcept;
			void initCamera()										noexcept;
			void initCommands()									noexcept;
			void initSyncPrimitives()						noexcept;

			void setupRenderPass()							noexcept;
			void setupFramebuffers()						noexcept;

		private:
			void resizeWindow()									noexcept;

		private:
			virtual void setupCommands()			noexcept = 0;
			virtual void draw()								noexcept;
			virtual void submitSceneToQueue()	noexcept;

		private:
			void setupRenderPassCommands(
				const VkCommandBuffer		&_cmdBuffer,
				const VkPipeline				&_pipeline,
				const VkPipelineLayout	&_pipelineLayout,
				const VkDescriptorSet		*_descSets,
				uint32_t								_descSetCount = 1
			)	noexcept;

		private:
			static void framebufferResize(GLFWwindow *_window, int _width, int _height) noexcept;
			static std::vector<const char*> getSurfaceExtensions() noexcept;

		protected:
			struct ScreenData
			{
				friend class Base;

				using ModelData = vk::Model::Data;
				using ModelDataList = vk::Vector<ModelData>;

				Camera													camera;
				ModelDataList										modelsData;

				bool														isInited 	= false;
				bool														isPaused	= false;
				bool														isResized	= false;

				private:
					using FramebufferData = vk::Framebuffer::Data<
						vk::Attachment::s_attCount
					>;
					using RenderPassData = vk::RenderPass::Data<
						vk::Attachment::s_attCount,
						vk::RenderPass::s_subpassCount,
						vk::RenderPass::s_spDepCount
					>;

					FramebufferData								framebufferData;
			 };

			std::unique_ptr<vk::Device>	m_device		= nullptr;
			GLFWwindow									*m_window		= nullptr;

		protected:
			ScreenData m_screenData;

		private:
			inline static Base *s_instance = nullptr;
	};
}