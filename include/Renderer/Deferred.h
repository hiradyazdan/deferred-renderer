#pragma once

#include "Base.h"

namespace renderer
{
	class Deferred final : public Base
	{
		friend class Base;

		public:
			~Deferred() final;

		public:
			void init()				noexcept override;
			void render()			noexcept override;
			void loadAssets() noexcept override;

		private:
			explicit Deferred(GLFWwindow *_window) : Base(_window) {}

		private:
			void initCmdBuffer()				noexcept;
			void initSyncPrimitive()		noexcept;

			void setupRenderPass()			noexcept;
			void setupFramebuffer()			noexcept;
			void setupUBOs()						noexcept;

			void setupDescriptors()			noexcept;
			void setupDescPool(
				uint32_t _materialCount,
				uint32_t _maxSetCount
			) noexcept;

			void setupPipelines()				noexcept;

			void setupRenderPassCommands(
				const VkCommandBuffer &_cmdBuffer
			)	noexcept;
			void submitOffscreenToQueue() noexcept;

			void setOffscreenUBOData(DeferredScreenData::OffScreenUBO &_data) 		noexcept;
			void setCompositionUBOData(DeferredScreenData::CompositionUBO &_data)	noexcept;
			void updateOffscreenUBO()			noexcept;
			void updateCompositionUBO() 	noexcept;

		private:
			void setupBaseCommands()	noexcept override;
			void setupCommands()			noexcept override;
			void onWindowResize()			noexcept override;
			void submitSceneToQueue()	noexcept override;
			void draw()								noexcept override;

		private:
			template<vk::Shader::Stage stage, uint16_t stageCount>
			void setShader(
				const char										*_shaderFile,
				vk::Shader::Data<stageCount>	&_data,
				VkSpecializationInfo					*_specializationInfo = nullptr
			) noexcept
			{
				auto &logicalDevice = m_device->getData().logicalDevice;
				auto &shaderModules = m_deferredScreenData.pipelineData.shaderModules;
				auto &module = _data.module;
				auto &modIndex = _data.moduleIndex;

				_data.stages[stage] = vk::Shader::load<stage, stageCount>(
					logicalDevice, constants::SHADERS_PATH + _shaderFile,
					module, _specializationInfo
				);

				if(_data.isValid())
				{
					shaderModules[modIndex] = module;
					modIndex++;
				}
			}

			template<vk::Pipeline::Type type, size_t shaderStageCount>
			void setPipeline(
				vk::Pipeline::PSO																							&_psoData,
				vk::Array<VkPipelineShaderStageCreateInfo, shaderStageCount>	&_shaderStages,
				uint16_t																											_index = 0
			)	noexcept
			{
				using Type = vk::Pipeline::Type;

				const auto isComposition = type == Type::COMPOSITION;
				auto index = isComposition
					? vk::toInt(type)
					: _index;
				auto &pipelineData = m_deferredScreenData.pipelineData;
				const auto &renderPass = isComposition
					? getRenderPass(m_screenData)
					: getRenderPass(m_deferredScreenData);

				vk::Pipeline::createGraphicsPipeline(
					m_device->getData().logicalDevice,
					renderPass,
					pipelineData.cache, pipelineData.layouts[0],
					_shaderStages,
					_psoData,
					pipelineData.pipelines[index]
				);
			}

		private:
			DeferredScreenData m_deferredScreenData;

			USE_DESC_SET_LAYOUT_BINDING_STRUCT(DeferredScreenData, DEFERRED_SHADING)
	};
}