#pragma once

#include "../vk.h"

#include "../Camera.h"
#include "../AssetHelper.h"

class GLFWwindow;

enum class vk::Model::Name : uint16_t
{
	SPONZA	= 0,
	_count_ = 1
};

inline const uint16_t vk::Attachment::s_attCount			= 2;
inline const uint16_t vk::RenderPass::s_subpassCount	= 1;
inline const uint16_t vk::RenderPass::s_spDepCount		= 2;
inline const uint16_t vk::Model			::s_modelCount		= vk::toInt(vk::Model::Name::_count_);

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
			virtual void init()	noexcept;
			virtual void render() noexcept = 0;

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

			void loadAsset(
				const std::string									&_fileName,
				std::vector<vk::Mesh>							&_meshes,
				float 														_scale = 1.0f
			) noexcept;

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

				bool														isInited 	= false;
				bool														isPaused	= false;
				bool														isResized	= false;

				vk::Array<vk::Model::Data, vk::Model::s_modelCount>	modelData;

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