#pragma once

#include "Base.h"

enum class vk::Attachment::Tag::Color : uint16_t
{
	POSITION	= 0,
	NORMAL		= 1,
	ALBEDO		= 2,
	_count_ = 3
};

enum class vk::Descriptor::LayoutCategory : uint16_t
{
	DEFERRED_SHADING = 0,
	_count_ = 1
};

enum class vk::Pipeline::Type : uint16_t
{
	COMPOSITION = 0, // lighting pass (deferred)
	OFFSCREEN		= 1, // geometry pass (g-buffer)
	_count_ = 2
};

enum class vk::Buffer::Category : uint16_t
{
	COMPOSITION = 0, // lighting pass (deferred)
	OFFSCREEN		= 1, // geometry pass (g-buffer)
	_count_ = 2
};

enum class vk::Texture::Sampler : uint16_t
{
	COLOR		= 0,
	NORMAL	= 1,
	_count_	= 2
};

enum class vk::Shader::Stage : uint16_t
{
	VERTEX		= 0,
	FRAGMENT	= 1,
	_count_ = 2
};

enum class vk::Model::ID : uint16_t
{
	SPONZA	= 0,
	_count_ = 1
};

inline const uint16_t vk::Buffer		::s_ubcCount						= vk::toInt(vk::Buffer		::Category			::_count_);
inline const uint16_t vk::Buffer		::s_bufferCount					= vk::Buffer::s_mbtCount + vk::Buffer::s_ubcCount;
inline const uint16_t vk::Descriptor::s_setLayoutCount			= vk::toInt(vk::Descriptor::LayoutCategory::_count_);
inline const uint16_t vk::Pipeline	::s_layoutCount					= 1;
inline const uint16_t vk::Pipeline	::s_pushConstCount			= 1;
inline const uint16_t vk::Model			::s_modelCount					= vk::toInt(vk::Model			::ID						::_count_);

static_assert(
	vk::Model::s_modelCount == constants::models.size(),
	"Model filenames and IDs should have an equal count."
);

namespace renderer
{
	class Deferred final : public Base
	{
		friend class Base;

		struct CompositionUBO
		{
			STACK_ONLY(CompositionUBO)

			struct Light
			{
				glm::vec4	position;
				glm::vec3	color;
				float			radius;
			};

			Light			lights[6];
			glm::vec4	viewPos;
		};
		struct OffScreenUBO
		{
			STACK_ONLY(OffScreenUBO)

			glm::mat4 projection;
			glm::mat4 view;
		};

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

			void setOffscreenUBOData(OffScreenUBO &_data) 		noexcept;
			void setCompositionUBOData(CompositionUBO &_data)	noexcept;
			void updateOffscreenUBO()			noexcept;
			void updateCompositionUBO() 	noexcept;

		private:
			void setupBaseCommands()	noexcept override;
			void setupCommands()			noexcept override;
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
				auto &shaderModules = m_offscreenData.pipelineData.shaderModules;
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
				auto &pipelineData = m_offscreenData.pipelineData;
				const auto &renderPass = isComposition
					? getScreenRenderPass(m_screenData)
					: getScreenRenderPass(m_offscreenData);

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
			struct OffScreenData : ScreenData
			{
				using Desc					= vk::Descriptor;
				using AttTag				= vk::Attachment::Tag;

				inline static const uint16_t s_fbAttCount		= vk::toInt(AttTag::Color::_count_) + 1;

				using DescriptorData	= Desc::Data<Desc::s_setLayoutCount>;
				using FramebufferData	= vk::Framebuffer::Data<s_fbAttCount>;
				using RenderPassData	= vk::RenderPass::Data<
					s_fbAttCount,
					vk::RenderPass::s_subpassCount,
					vk::RenderPass::s_spDepCount
				>;
				using PipelineData		= vk::Pipeline::Data<
				  constants::shaders::_count_,
					vk::Pipeline::s_layoutCount,
					vk::Pipeline::s_pushConstCount
				>;
				using BufferData			= vk::Buffer	::Data<
				  vk::Buffer::Type::ANY,
					vk::Buffer::s_bufferCount
				>;

				FramebufferData	framebufferData;
				PipelineData		pipelineData;
				DescriptorData	descriptorData;
				BufferData			bufferData; // Inclusive for both UBOs & Model buffers

				VkCommandBuffer	cmdBuffer	= VK_NULL_HANDLE;
				VkSemaphore			semaphore	= VK_NULL_HANDLE;

				BEGIN_DESC_SET_LAYOUT_BINDING_STRUCT(DEFERRED_SHADING)

					LAYOUT_BINDING_UNIFORM_BUFFER(GEOM_VS_UBO, StageFlag::VERTEX)					// VS uniform buffer
					LAYOUT_BINDING_COMBINED_IMAGE_SAMPLER(POSITION, StageFlag::FRAGMENT)	// Position / Color map
					LAYOUT_BINDING_COMBINED_IMAGE_SAMPLER(NORMAL, StageFlag::FRAGMENT)		// Normals  / Normal Map
					LAYOUT_BINDING_COMBINED_IMAGE_SAMPLER(ALBEDO, StageFlag::FRAGMENT)		// Albedo
					LAYOUT_BINDING_UNIFORM_BUFFER(LIGHT_FS_UBO, StageFlag::FRAGMENT)			// FS uniform buffer

				END_DESC_SET_LAYOUT_BINDING_STRUCT()
			} m_offscreenData;

			USE_DESC_SET_LAYOUT_BINDING_STRUCT(OffScreenData, DEFERRED_SHADING)
	};
}