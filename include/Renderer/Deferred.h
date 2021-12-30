#pragma once

#include "Base.h"

enum class vk::Pipeline::Type
{
	COMPOSITION = 0, // lighting pass (deferred)
	OFFSCREEN		= 1, // geometry pass (g-buffer)
	_count_ = 2
};

enum class vk::Shader::Stage
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
			void init(Window::WindowData &_winData) noexcept override;

		private:
			explicit Deferred(GLFWwindow *_window) : Base(_window) {}

		private:
			void initCmdBuffer()				noexcept;
			void initSyncObject()				noexcept;

			void setupCommands()				noexcept;
			void setupRenderPassCommands(
				const VkExtent2D					&_swapchainExtent,
				const VkCommandBuffer			&_cmdBuffer
			)	noexcept;
			void setupPipelines()				noexcept;

			template<vk::Shader::Stage stage>
			vk::Shader::Data setShader(const char *_shaderPath, vk::Shader::Data &_data) noexcept
			{
				auto &deviceData = m_device->getData();
				auto &logicalDevice = deviceData.logicalDevice;
				auto &shaderModules = m_offscreenData.pipelineData.shaderModules;

				vk::Shader::load<stage>(logicalDevice, _shaderPath, _data);

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

				auto &deviceData = m_device->getData();
				auto &framebufferData = m_offscreenData.renderPassData.framebufferData;
				auto &pipelineData = m_offscreenData.pipelineData;

				vk::Pipeline::createGraphicsPipeline<shaderStageCount>(
					deviceData.logicalDevice,
					_useScreenRenderPass ? getScreenRenderPass() : framebufferData.renderPass,
					pipelineData.cache,
					pipelineData.layout,
					_shaderStages,
					_psoData,
					pipelineData.pipelines[pipelineIndex]
				);
			}

		private:
			void submitOffscreenQueue() noexcept;
			void submitSceneQueue()			noexcept override;
			void draw()									noexcept override;

		private:
			inline static Deferred *s_instance = nullptr;

			struct OffScreenData : ScreenData
			{
				inline static const uint16_t s_fbAttCount				= 4;
				inline static const uint16_t s_subpassCount			= 1;
				inline static const uint16_t s_spDepCount				= 2;

				inline static const uint16_t s_pipelineCount		= static_cast<uint16_t>(vk::Pipeline::Type::_count_);
				inline static const uint16_t s_shaderStageCount = static_cast<uint16_t>(vk::Shader::Stage::_count_);

				using RenderPassData = vk::RenderPass::Data<
					s_fbAttCount,
					s_subpassCount,
					s_spDepCount
				>;

				RenderPassData											renderPassData;
				vk::Pipeline::Data<s_pipelineCount>	pipelineData;

				VkCommandBuffer			cmdBuffer	= VK_NULL_HANDLE;
				VkSemaphore					semaphore	= VK_NULL_HANDLE;
			} m_offscreenData;
	};
}