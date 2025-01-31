#pragma once

#include <vulkan/vulkan.h>

#include "_constants.h"

namespace vk
{
	using NOOP	= constants::NOOP;
	using LogicalDevice = VkDevice;

	struct LoadOp : NOOP
	{
		static constexpr const VkAttachmentLoadOp CLEAR     = VK_ATTACHMENT_LOAD_OP_CLEAR;
		static constexpr const VkAttachmentLoadOp DONT_CARE = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	};

	struct StoreOp : NOOP
	{
		static constexpr const VkAttachmentStoreOp STORE     = VK_ATTACHMENT_STORE_OP_STORE;
		static constexpr const VkAttachmentStoreOp DONT_CARE = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	};

	struct FormatType : NOOP
	{
		static constexpr const VkFormat D32_SFLOAT_S8_UINT  	= VK_FORMAT_D32_SFLOAT_S8_UINT;
		static constexpr const VkFormat D32_SFLOAT          	= VK_FORMAT_D32_SFLOAT;
		static constexpr const VkFormat D24_UNORM_S8_UINT   	= VK_FORMAT_D24_UNORM_S8_UINT;
		static constexpr const VkFormat D16_UNORM_S8_UINT			= VK_FORMAT_D16_UNORM_S8_UINT;
		static constexpr const VkFormat D16_UNORM							= VK_FORMAT_D16_UNORM;

		static constexpr const VkFormat B8G8R8_SRGB						= VK_FORMAT_B8G8R8_SRGB;
		static constexpr const VkFormat B8G8R8A8_SRGB					= VK_FORMAT_B8G8R8A8_SRGB;
		static constexpr const VkFormat R8G8B8_SRGB						= VK_FORMAT_R8G8B8_SRGB;
		static constexpr const VkFormat R8G8B8A8_SRGB      		= VK_FORMAT_R8G8B8A8_SRGB;
		static constexpr const VkFormat A8B8G8R8_SRGB_PACK32	= VK_FORMAT_A8B8G8R8_SRGB_PACK32;

		static constexpr const VkFormat B8G8R8_UNORM					= VK_FORMAT_B8G8R8_UNORM;
		static constexpr const VkFormat B8G8R8A8_UNORM				= VK_FORMAT_B8G8R8A8_UNORM;
		static constexpr const VkFormat R8G8B8_UNORM					= VK_FORMAT_R8G8B8_UNORM;
		static constexpr const VkFormat R8G8B8A8_UNORM      	= VK_FORMAT_R8G8B8A8_UNORM;
		static constexpr const VkFormat A8B8G8R8_UNORM_PACK32	= VK_FORMAT_A8B8G8R8_UNORM_PACK32;

		static constexpr const VkFormat R32G32B32_SFLOAT			= VK_FORMAT_R32G32B32_SFLOAT;
		static constexpr const VkFormat R32G32_SFLOAT					= VK_FORMAT_R32G32_SFLOAT;
		static constexpr const VkFormat R32G32B32A32_SFLOAT		= VK_FORMAT_R32G32B32A32_SFLOAT;
	};

	struct ColorSpace : NOOP
	{
		static constexpr const VkColorSpaceKHR SRGB_NONLINEAR = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	};

	namespace image
	{
		struct SampleCountFlag : NOOP
		{
			static constexpr const VkSampleCountFlagBits _1       = VK_SAMPLE_COUNT_1_BIT;
			static constexpr const VkSampleCountFlagBits _2       = VK_SAMPLE_COUNT_2_BIT;
			static constexpr const VkSampleCountFlagBits _4       = VK_SAMPLE_COUNT_4_BIT;
			static constexpr const VkSampleCountFlagBits _8       = VK_SAMPLE_COUNT_8_BIT;
			static constexpr const VkSampleCountFlagBits _16      = VK_SAMPLE_COUNT_16_BIT;
			static constexpr const VkSampleCountFlagBits _32      = VK_SAMPLE_COUNT_32_BIT;
			static constexpr const VkSampleCountFlagBits _64      = VK_SAMPLE_COUNT_64_BIT;
			static constexpr const VkSampleCountFlagBits MAX_ENUM = VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM;
		};

		struct UsageFlag : NOOP
		{
			static constexpr const VkImageUsageFlagBits COLOR_ATTACHMENT         = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			static constexpr const VkImageUsageFlagBits DEPTH_STENCIL_ATTACHMENT = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			static constexpr const VkImageUsageFlagBits INPUT_ATTACHMENT         = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
			static constexpr const VkImageUsageFlagBits TRANSFER_DST             = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		};

		struct LayoutType : NOOP
		{
			static constexpr const VkImageLayout UNDEFINED                         = VK_IMAGE_LAYOUT_UNDEFINED;
			static constexpr const VkImageLayout PRESENT_SRC_KHR                   = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			static constexpr const VkImageLayout COLOR_ATTACHMENT_OPTIMAL          = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			static constexpr const VkImageLayout SHADER_READ_ONLY_OPTIMAL          = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			static constexpr const VkImageLayout DEPTH_STENCIL_ATTACHMENT_OPTIMAL  = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		};
	}

	namespace shader
	{
		struct StageFlag : NOOP
		{
			static constexpr const VkShaderStageFlagBits VERTEX                   = VK_SHADER_STAGE_VERTEX_BIT;
			static constexpr const VkShaderStageFlagBits FRAGMENT                 = VK_SHADER_STAGE_FRAGMENT_BIT;
			static constexpr const VkShaderStageFlagBits COMPUTE                  = VK_SHADER_STAGE_COMPUTE_BIT;
			static constexpr const VkShaderStageFlagBits GEOMETRY                 = VK_SHADER_STAGE_GEOMETRY_BIT;
			static constexpr const VkShaderStageFlagBits TESSELLATION_CONTROL     = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
			static constexpr const VkShaderStageFlagBits TESSELLATION_EVALUATION  = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
			static constexpr const VkShaderStageFlagBits RAYGEN_KHR               = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
			static constexpr const VkShaderStageFlagBits ANY_HIT_KHR              = VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
			static constexpr const VkShaderStageFlagBits CLOSEST_HIT_KHR          = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
			static constexpr const VkShaderStageFlagBits MISS_KHR                 = VK_SHADER_STAGE_MISS_BIT_KHR;
			static constexpr const VkShaderStageFlagBits INTERSECTION_KHR         = VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
			static constexpr const VkShaderStageFlagBits CALLABLE_KHR             = VK_SHADER_STAGE_CALLABLE_BIT_KHR;
		};
	}

	namespace descriptor
	{
		struct Type : NOOP
		{
			static constexpr const VkDescriptorType SAMPLER                 = VK_DESCRIPTOR_TYPE_SAMPLER;
			static constexpr const VkDescriptorType COMBINED_IMAGE_SAMPLER  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			static constexpr const VkDescriptorType SAMPLED_IMAGE           = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			static constexpr const VkDescriptorType STORAGE_IMAGE           = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			static constexpr const VkDescriptorType UNIFORM_TEXEL_BUFFER    = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
			static constexpr const VkDescriptorType STORAGE_TEXEL_BUFFER    = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
			static constexpr const VkDescriptorType UNIFORM_BUFFER          = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			static constexpr const VkDescriptorType STORAGE_BUFFER          = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			static constexpr const VkDescriptorType UNIFORM_BUFFER_DYNAMIC  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			static constexpr const VkDescriptorType STORAGE_BUFFER_DYNAMIC  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
			static constexpr const VkDescriptorType INPUT_ATTACHMENT        = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		};
	}
}