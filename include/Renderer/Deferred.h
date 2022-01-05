#pragma once

#include "Base.h"

// TODO: are these definitions implementation details?

enum class vk::Attachment::Tag::Color : uint16_t
{
	POSITION	= 0,
	NORMAL		= 1,
	ALBEDO		= 2,
	_count_ = 3
};

enum class vk::Pipeline::Type : uint16_t
{
	COMPOSITION = 0, // lighting pass (deferred)
	OFFSCREEN		= 1, // geometry pass (g-buffer)
	_count_ = 2
};

enum class vk::Buffer::Type : uint16_t
{
	COMPOSITION = 0, // lighting pass (deferred)
	OFFSCREEN		= 1, // geometry pass (g-buffer)
	_count_ = 2
};

enum class vk::Shader::Stage : uint16_t
{
	VERTEX		= 0,
	FRAGMENT	= 1,
	_count_ = 2
};

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
			void init()		noexcept override;
			void render()	noexcept override;

		private:
			explicit Deferred(GLFWwindow *_window) : Base(_window) {}

		private:
			void initCmdBuffer()				noexcept;
			void initSyncObject()				noexcept;

			void setupRenderPass()			noexcept;
			void setupFramebuffer()			noexcept;
			void setupUBOs()						noexcept;

			void setupDescriptors()			noexcept;
			void setDescPool()					noexcept;
			void setDescSetLayout(
				std::vector<VkDescriptorSetLayoutBinding> &_layoutBindings,
				uint32_t																	_index = 0
			)	noexcept;

			void setupPipelines()				noexcept;

			void setupDeferredCommands()		noexcept;
			void setupRenderPassCommands(
				const VkCommandBuffer &_cmdBuffer
			)	noexcept;
			void submitOffscreenQueue() noexcept;

		private:
			void setupCommands()				noexcept override;
			void submitSceneQueue()			noexcept override;
			void draw()									noexcept override;

		private:
			template<vk::Shader::Stage stage>
			vk::Shader::Data setShader(
				const char				*_shaderFile,
				vk::Shader::Data	&_data
			) noexcept
			{
				auto &logicalDevice = m_device->getData().logicalDevice;
				auto &shaderModules = m_offscreenData.pipelineData.shaderModules;

				vk::Shader::load<stage>(logicalDevice, _shaderFile, _data);

				if(_data.isValid())
				{
					shaderModules.push_back(_data.module);
				}

				return _data;
			}

			template<vk::Pipeline::Type type, uint16_t shaderStageCount>
			void setPipeline(
				vk::Pipeline::PSO																							&_psoData,
				std::array<VkPipelineShaderStageCreateInfo, shaderStageCount>	&_shaderStages,
				bool																													_useScreenRenderPass = true
			)	noexcept
			{
				const auto &pipelineIndex = static_cast<int>(type);

				auto &framebufferData = m_offscreenData.renderPassData.framebufferData;
				auto &pipelineData = m_offscreenData.pipelineData;
				auto &renderPass = _useScreenRenderPass ? getScreenRenderPass() : framebufferData.renderPass;

				vk::Pipeline::createGraphicsPipeline<shaderStageCount>(
					m_device->getData().logicalDevice,
					renderPass,
					pipelineData.cache,
					pipelineData.layouts[0],
					_shaderStages,
					_psoData,
					pipelineData.pipelines[pipelineIndex]
				);
			}

		private:
			inline static Deferred *s_instance = nullptr;

			struct OffScreenData : ScreenData
			{
				using AttTag = vk::Attachment::Tag;

				inline static const uint16_t s_fbAttCount					= static_cast<uint16_t>(AttTag::Color::_count_) + 1;
				inline static const uint16_t s_subpassCount				= 1;
				inline static const uint16_t s_spDepCount					= 2;

				inline static const uint16_t s_descSetLayoutCount = 1;
				inline static const uint16_t s_pipelineCount			= static_cast<uint16_t>(vk::Pipeline::Type	::_count_);
				inline static const uint16_t s_uboCount						= static_cast<uint16_t>(vk::Buffer	::Type	::_count_);
				inline static const uint16_t s_shaderStageCount 	= static_cast<uint16_t>(vk::Shader	::Stage	::_count_);

				using RenderPassData = vk::RenderPass::Data<
					s_fbAttCount,
					s_subpassCount,
					s_spDepCount
				>;
				using PipelineData		= vk::Pipeline	::Data<s_pipelineCount>;
				using DescriptorData	= vk::Descriptor::Data<s_descSetLayoutCount>;
				using BufferData			= vk::Buffer		::Data<s_uboCount>;

				RenderPassData	renderPassData;
				PipelineData		pipelineData;
				DescriptorData	descriptorData;
				BufferData			bufferData;

				VkCommandBuffer	cmdBuffer	= VK_NULL_HANDLE;
				VkSemaphore			semaphore	= VK_NULL_HANDLE;
			} m_offscreenData;
	};
}