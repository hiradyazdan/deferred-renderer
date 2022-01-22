#pragma once

#include "VkData.h"
#include "../Camera.h"

// CPU / GAPI Data

namespace renderer
{
	struct ScreenData
	{
		friend class Base;

		using ModelData				= vk::Model::Data;
		using TextureData			= vk::Texture::Data;
		using ModelDataList		= vk::Vector<ModelData>;
		using TextureDataList	= vk::Vector<TextureData>;

		Camera													camera;
		ModelDataList										modelsData;
		TextureDataList									texturesData;

		bool														isInited 	= false;
		bool														isUpdated = false;
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

	struct DeferredScreenData : ScreenData
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
	};
}