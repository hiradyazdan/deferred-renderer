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

enum class vk::Descriptor::LayoutBinding : uint16_t
{
	VS_UBO		= 0,
	POSITION	= vk::toInt(vk::Attachment::Tag::Color::POSITION)	+ 1,
	NORMAL		= vk::toInt(vk::Attachment::Tag::Color::NORMAL)		+ 1,
	ALBEDO		= vk::toInt(vk::Attachment::Tag::Color::ALBEDO)		+ 1,
	FS_UBO		= 4,
	_count_ = 5
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

inline const uint16_t vk::Pipeline	::s_pipelineCount				= vk::toInt(vk::Pipeline	::Type					::_count_);
inline const uint16_t vk::Buffer		::s_ubcCount						= vk::toInt(vk::Buffer		::Category			::_count_);
inline const uint16_t vk::Buffer		::s_bufferCount					= vk::Buffer::s_ubcCount + vk::Buffer::s_mbtCount;
inline const uint16_t vk::Texture		::s_samplerCount				= vk::toInt(vk::Texture		::Sampler				::_count_);
inline const uint16_t vk::Descriptor::s_layoutBindingCount	= vk::toInt(vk::Descriptor::LayoutBinding	::_count_);
inline const uint16_t vk::Descriptor::s_setLayoutCount			= vk::toInt(vk::Descriptor::LayoutCategory::_count_);
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

		public:
			~Deferred() final = default; // TODO: clean up vk resources

		public:
			void init()				noexcept override;
			void render()			noexcept override;
			void loadAssets() noexcept override;

		private:
			explicit Deferred(GLFWwindow *_window) : Base(_window) {}

		private:
			void initCmdBuffer()				noexcept;
			void initSyncObject()				noexcept;

			void setupRenderPass()			noexcept;
			void setupFramebuffer()			noexcept;
			void setupUBOs()						noexcept;

			void setupDescriptors()			noexcept;

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

			template<vk::Pipeline::Type type, uint16_t shaderStageCount>
			void setPipeline(
				vk::Pipeline::PSO																							&_psoData,
				vk::Array<VkPipelineShaderStageCreateInfo, shaderStageCount>	&_shaderStages
			)	noexcept
			{
				auto &framebufferData = m_offscreenData.renderPassData.framebufferData;
				auto &pipelineData = m_offscreenData.pipelineData;
				auto &renderPass = type == vk::Pipeline::Type::COMPOSITION
					? getScreenRenderPass()
					: framebufferData.renderPass;

				vk::Pipeline::createGraphicsPipeline<shaderStageCount>(
					m_device->getData().logicalDevice,
					renderPass,
					pipelineData.cache,
					pipelineData.layouts[0],
					_shaderStages,
					_psoData,
					pipelineData.pipelines[type]
				);
			}

		private:
			struct OffScreenData : ScreenData
			{
				using Desc					= vk::Descriptor;
				using AttTag				= vk::Attachment::Tag;

				inline static const uint16_t s_fbAttCount		= vk::toInt(AttTag::Color::_count_) + 1;

				using DescriptorData	= vk::Descriptor::Data<
					Desc::s_layoutBindingCount,
					Desc::s_setLayoutCount
				>;
				using RenderPassData = vk::RenderPass::Data<
					s_fbAttCount,
					vk::RenderPass::s_subpassCount,
					vk::RenderPass::s_spDepCount
				>;
				using PipelineData		= vk::Pipeline	::Data<
				  vk::Pipeline::s_pipelineCount,
					vk::Pipeline::s_pipelineCount + vk::toInt(vk::Shader::Stage::_count_)
				>;
				using BufferData			= vk::Buffer	::Data<vk::Buffer::Type::ANY, vk::Buffer::s_bufferCount>;
				using TextureData 		= vk::Texture	::Data<vk::Texture::s_samplerCount>;

				RenderPassData	renderPassData;
				PipelineData		pipelineData;
				DescriptorData	descriptorData;
				BufferData			bufferData; // Inclusive for both UBOs & Model buffers
				TextureData			textureData;

				VkCommandBuffer	cmdBuffer	= VK_NULL_HANDLE;
				VkSemaphore			semaphore	= VK_NULL_HANDLE;
			} m_offscreenData;
	};
}