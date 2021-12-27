#pragma once

#include "utils.h"
#include "Image.h"

namespace vk
{
	class Attachment
	{
		friend class Framebuffer;
		friend class RenderPass;

		template<uint16_t attCount>
		struct Data
		{
			enum class Type
			{
				FRAMEBUFFER,
				COLOR,
				DEPTH,
				INPUT
			};

			struct AttSubpassMap
			{
				Type type;
				std::vector<uint16_t> indices = { 0 };
			};

			std::array<AttSubpassMap, attCount> attSpMaps = {
				{
					{ Type::FRAMEBUFFER,	std::vector<uint16_t>({ 0 }) },
					{ Type::DEPTH,				std::vector<uint16_t>({ 0 }) }
				}
			};
			std::array<VkFormat, attCount> formats;

			std::array<VkAttachmentDescription, attCount> descs;

			std::array<VkImage,									attCount> images;
			std::array<VkImageView,							attCount> imageViews;
			std::array<VkDeviceMemory,					attCount> imageMemories;

			std::array<VkClearValue,						attCount> clearValues;
		};

		private:
			inline static void setFramebufferAttachment(
				VkAttachmentDescription &_desc, VkClearValue	&_clearValue,
				uint32_t								_index,
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

				_clearValue.color = (VkClearColorValue&&) constants::CLEAR_COLOR;
			}

			inline static void setColorAttachment(
				const VkExtent2D					&_swapchainExtent,	const VkPhysicalDeviceMemoryProperties	&_memProps,
				const VkDevice						&_logicalDevice,		const VkPhysicalDevice									&_physicalDevice,
				VkAttachmentDescription		&_desc,		VkClearValue		&_clearValue,
				VkImage										&_image,	VkDeviceMemory	&_imageMemory,	VkImageView	&_imageView,
				uint32_t									_index,
				const VkFormat						&_format = FormatType::R8G8B8A8_UNORM,
				const VkImageAspectFlags	&_aspectMask = VK_IMAGE_ASPECT_COLOR_BIT
			) noexcept
			{
				_desc.flags           = 0;
				_desc.format          = _format;
				_desc.samples         = image::SampleCountFlag	::_1;
				_desc.loadOp          = LoadOp          				::DONT_CARE;
				_desc.storeOp         = StoreOp         				::STORE;
				_desc.stencilLoadOp   = LoadOp          				::DONT_CARE;
				_desc.stencilStoreOp  = StoreOp         				::DONT_CARE;
				_desc.initialLayout   = image::LayoutType				::UNDEFINED;
				_desc.finalLayout     = image::LayoutType				::SHADER_READ_ONLY_OPTIMAL;

				_clearValue.color = (VkClearColorValue&&) constants::CLEAR_COLOR;

				setImageData(
					_format, image::UsageFlag::COLOR_ATTACHMENT,
					_swapchainExtent, _memProps,
					_logicalDevice, _physicalDevice,
					_aspectMask,
					_image,
					_imageMemory,
					_imageView
				);
			}

			inline static void setDepthAttachment(
				const VkExtent2D					&_swapchainExtent,	const VkPhysicalDeviceMemoryProperties	&_memProps,
				const VkDevice						&_logicalDevice,		const VkPhysicalDevice									&_physicalDevice,
				VkAttachmentDescription		&_desc,		VkClearValue		&_clearValue,
				VkImage										&_image,	VkDeviceMemory	&_imageMemory,	VkImageView	&_imageView,
				uint32_t									_index,
				const VkFormat						&_format,
				const VkImageAspectFlags	&_aspectMask
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

				_clearValue.color = (VkClearColorValue&&) constants::CLEAR_DEPTH_STENCIL;

				setImageData(
					_format, image::UsageFlag::DEPTH_STENCIL_ATTACHMENT,
					_swapchainExtent, _memProps,
					_logicalDevice, _physicalDevice,
					_aspectMask,
					_image,
					_imageMemory,
					_imageView
				);
			}

			inline static void setInputAttachment(
				const VkExtent2D				&_swapchainExtent,	const VkPhysicalDeviceMemoryProperties	&_memProps,
				const VkDevice					&_logicalDevice,		const VkPhysicalDevice									&_physicalDevice,
				VkAttachmentDescription	&_desc,		VkClearValue		&_clearValue,
				VkImage									&_image,	VkDeviceMemory	&_imageMemory,	VkImageView	&_imageView,
				uint32_t								_index,
				const VkFormat					&_format
			) noexcept
			{
				// TODO
			}

			inline static void setImageData(
				const VkFormat		&_format, 					const VkImageUsageFlagBits							&_usage,
				const VkExtent2D	&_swapchainExtent,	const VkPhysicalDeviceMemoryProperties	&_memProps,
				const VkDevice		&_logicalDevice,		const VkPhysicalDevice									&_physicalDevice,
				const VkImageAspectFlags &_aspectMask,
				VkImage 					&_image,
				VkDeviceMemory		&_imageMemory,
				VkImageView				&_imageView
			) noexcept
			{
				vk::Image::createImage(
					_logicalDevice, _swapchainExtent,
					_format, VK_IMAGE_TILING_OPTIMAL, _usage,
					_image
				);
				vk::Image::createImageMemory(
					_logicalDevice, _physicalDevice, _memProps,
					_image,
					_imageMemory
				);
				vk::Image::createImageView(
					_logicalDevice, _image, _format,
					_imageView,
					_aspectMask
				);
			}
	};
}