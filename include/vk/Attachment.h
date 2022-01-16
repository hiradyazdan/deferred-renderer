#pragma once

#include "Image.h"

namespace vk
{
	class Attachment
	{
		friend class Framebuffer;
		friend class RenderPass;

		public:
			static const uint16_t s_attCount;

		public:
			struct Tag : NOOP
			{
				enum class Color : uint16_t;
				enum class Input : uint16_t;
			};

			enum class Type
			{
				FRAMEBUFFER	= 0,
				DEPTH				= 1,
				COLOR				= 2,
				INPUT			 	= 3
			};

			// many to many
			struct AttSubpassMap
			{
				STACK_ONLY(AttSubpassMap);

				Type attType = Type(0);
				std::vector<uint16_t> spIndices = { 0 };
			};

			template<uint16_t attCount>
			struct Data
			{
				struct Temp
				{
					STACK_ONLY(Temp);

					Array<VkDescriptorImageInfo,	attCount> imageDescs;
				};

				Array<VkFormat,									attCount> formats;
				Array<VkAttachmentDescription,	attCount> descs;
				Array<VkImage,									attCount> images;
				Array<VkImageView,							attCount> imageViews;
				Array<VkDeviceMemory,						attCount> imageMemories;
				Array<VkClearValue,							attCount> clearValues;

				uint16_t																	depthAttIndex = 0;

				VkExtent2D																extent = { 2048, 2048 };
				std::vector<VkSampler>										samplers;
			};

		private:
			inline static void setFramebufferAttachment(
				VkAttachmentDescription &_desc, VkClearValue	&_clearValue,
				const VkFormat					&_format = FormatType::B8G8R8A8_SRGB
			) noexcept
			{
				_desc.flags           = 0;
				_desc.format          = _format;
				_desc.samples         = image::SampleCountFlag	::_1;
				_desc.loadOp          = LoadOp          				::CLEAR;
				_desc.storeOp         = StoreOp         				::STORE;
				_desc.stencilLoadOp   = LoadOp          				::DONT_CARE;
				_desc.stencilStoreOp  = StoreOp         				::DONT_CARE;
				_desc.initialLayout   = image::LayoutType     	::UNDEFINED;
				_desc.finalLayout     = image::LayoutType     	::PRESENT_SRC_KHR;

				_clearValue.color = { (VkClearColorValue&&) constants::CLEAR_COLOR };
			}

			inline static void setColorAttachment(
				const VkExtent2D					&_extent,					const VkPhysicalDeviceMemoryProperties	&_memProps,
				const VkDevice						&_logicalDevice,
				VkAttachmentDescription		&_desc,		VkClearValue		&_clearValue,
				VkImage										&_image,	VkDeviceMemory	&_imageMemory,	VkImageView	&_imageView,
				const VkFormat						&_format			= FormatType::R8G8B8A8_UNORM,
				const VkImageAspectFlags	&_aspectMask	= VK_IMAGE_ASPECT_COLOR_BIT,
				const VkImageUsageFlags		&_usage				= image::UsageFlag::COLOR_ATTACHMENT | VK_IMAGE_USAGE_SAMPLED_BIT
			) noexcept
			{
				_desc.flags           = 0;
				_desc.format          = _format;
				_desc.samples         = image::SampleCountFlag	::_1;
				_desc.loadOp          = LoadOp          				::CLEAR;
				_desc.storeOp         = StoreOp         				::STORE;
				_desc.stencilLoadOp   = LoadOp          				::DONT_CARE;
				_desc.stencilStoreOp  = StoreOp         				::DONT_CARE;
				_desc.initialLayout   = image::LayoutType				::UNDEFINED;
				_desc.finalLayout     = image::LayoutType				::SHADER_READ_ONLY_OPTIMAL;

				_clearValue.color = { (VkClearColorValue&&) constants::CLEAR_COLOR };

				setImageData(
					_logicalDevice,
					_format, _usage,
					_extent, _memProps,
					_aspectMask,
					_image, _imageMemory, _imageView
				);
			}

			inline static void setDepthAttachment(
				const VkExtent2D					&_extent,					const VkPhysicalDeviceMemoryProperties	&_memProps,
				const VkDevice						&_logicalDevice,
				VkAttachmentDescription		&_desc,		VkClearValue		&_clearValue,
				VkImage										&_image,	VkDeviceMemory	&_imageMemory,	VkImageView	&_imageView,
				const VkFormat						&_format,
				const VkImageAspectFlags	&_aspectMask	= VK_IMAGE_ASPECT_DEPTH_BIT,
				const VkImageUsageFlags		&_usage				= image::UsageFlag::DEPTH_STENCIL_ATTACHMENT
			) noexcept
			{
				_desc.flags           = 0;
				_desc.format          = _format;
				_desc.samples         = image::SampleCountFlag	::_1;
				_desc.loadOp          = LoadOp          				::CLEAR;
				_desc.storeOp         = StoreOp         				::STORE;
				_desc.stencilLoadOp   = LoadOp          				::CLEAR;
				_desc.stencilStoreOp  = StoreOp         				::DONT_CARE;
				_desc.initialLayout   = image::LayoutType				::UNDEFINED;
				_desc.finalLayout     = image::LayoutType				::DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

				_clearValue.depthStencil = (VkClearDepthStencilValue&&) constants::CLEAR_DEPTH_STENCIL;

				setImageData(
					_logicalDevice,
					_format, _usage,
					_extent, _memProps,
					_aspectMask,
					_image, _imageMemory, _imageView
				);
			}

			inline static void setInputAttachment(
				const VkExtent2D				&_extent,					const VkPhysicalDeviceMemoryProperties	&_memProps,
				const VkDevice					&_logicalDevice,
				VkAttachmentDescription	&_desc,		VkClearValue		&_clearValue,
				VkImage									&_image,	VkDeviceMemory	&_imageMemory,	VkImageView	&_imageView,
				const VkFormat					&_format
			) noexcept
			{
				// TODO
			}

			inline static void setImageData(
				const VkDevice		&_logicalDevice,
				const VkFormat		&_format, const VkImageUsageFlags									&_usage,
				const VkExtent2D	&_extent, const VkPhysicalDeviceMemoryProperties	&_memProps,
				const VkImageAspectFlags &_aspectMask,
				VkImage &_image, VkDeviceMemory &_imageMemory, VkImageView &_imageView
			) noexcept
			{
				Image::create(
					_logicalDevice, _extent,
					_format, VK_IMAGE_TILING_OPTIMAL, _usage,
					_image
				);
				Image::createMemory(
					_logicalDevice, _memProps,
					_image,
					_imageMemory
				);
				Image::createImageView(
					_logicalDevice, _image, _format,
					_imageView,
					1, 1, 0,
					_aspectMask
				);
			}

		private:
			inline static void destroy(
				const VkDevice				&_logicalDevice,
				const VkImageView			&_imageView,
				const VkImage 				&_image,
				const VkDeviceMemory	&_imageMemory
			) noexcept
			{
				vkDestroyImageView(_logicalDevice,	_imageView,		nullptr);
				vkDestroyImage		(_logicalDevice,	_image,				nullptr);
				vkFreeMemory			(_logicalDevice,	_imageMemory,	nullptr);
			}
	};
}