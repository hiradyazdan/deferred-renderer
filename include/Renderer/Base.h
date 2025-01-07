#pragma once

#include "ScreenData.h"
#include "../AssetHelper.h"

class GLFWwindow;

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
			virtual ~Base();

		public:
			virtual void init()				noexcept;
			virtual void render()			noexcept = 0;
			virtual void loadAssets()	noexcept = 0;

		protected:
			explicit Base(GLFWwindow *_window)
			: m_window(_window)
			, m_device(std::make_unique<vk::Device>())
			, m_screenData() {}

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
			  vk::Model::ID			modelId,
				vk::Buffer::Type	type,
				uint16_t					bufferCount
			>
			void loadAsset(
				vk::Buffer::Data<type, bufferCount>	&_bufferData,
				float 															_scale = 1.0f
			) noexcept
			{
				auto &model = m_screenData.modelsData[modelId];
				auto &modelTextures = m_screenData.texturesData[modelId];
				auto &modelFile = constants::models[modelId];
				auto modelTexDir = std::string(modelFile).substr(0, std::string(modelFile).find('.'));

				AssetHelper::load(
					m_device,
					constants::MODELS_PATH + modelFile,
					model, modelTextures,
					_scale, constants::TEXTURES_PATH + modelTexDir + "/"
				);
				vk::Model::setup(m_device, model, _bufferData);
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
			inline static auto getRenderPass(const TScreenData &_screenData) noexcept
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
			virtual void setupBaseCommands()  noexcept = 0;
			virtual void setupCommands()			noexcept = 0;
			virtual void onWindowResize()			noexcept {};
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
			static void resizeScreen(GLFWwindow *_window, int, int) noexcept;
			static std::vector<const char*> getSurfaceExtensions() noexcept;

		protected:
			std::unique_ptr<vk::Device>	m_device		= nullptr;
			GLFWwindow									*m_window		= nullptr;

		public:
			Camera &getCamera() noexcept { return m_screenData.camera; }

		protected:
			ScreenData m_screenData;

		private:
			inline static Base *s_instance = nullptr;
	};
}