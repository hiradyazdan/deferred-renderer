#pragma once

#include "Attachment.h"

namespace vk
{
	class Framebuffer
	{
		friend class RenderPass;
		friend class Command;

		template<uint16_t attCount>
		struct Data
		{
			Attachment::Data<attCount>	attachments;

			VkFramebuffer	framebuffer	= VK_NULL_HANDLE;
			VkRenderPass	renderPass	= VK_NULL_HANDLE;
		};

		public:
			template<uint16_t attCount>
			static void createAttachments(
				const VkDevice							&_logicalDevice,		const VkPhysicalDevice									&_physicalDevice,
				const VkExtent2D						&_swapchainExtent,	const VkPhysicalDeviceMemoryProperties	&_memProps,
				Attachment::Data<attCount>	&_attachmentsData,
				bool												_isDefault = true
			) noexcept
			{
				auto &attSpMaps     = _attachmentsData.attSpMaps;
				auto &descs         = _attachmentsData.descs;
				auto &formats				= _attachmentsData.formats;
				auto &clearValues   = _attachmentsData.clearValues;

				auto &images        = _attachmentsData.images;
				auto &imageMemories = _attachmentsData.imageMemories;
				auto &imageViews    = _attachmentsData.imageViews;

				using AttType = typename Attachment::Data<attCount>::Type;

				uint32_t attIndex = 0;
				for(const auto &attSpMap : attSpMaps)
				{
					auto &type								= attSpMap.type;
					auto &desc                = descs[attIndex];
					auto &format							= formats[attIndex];
					auto &clearValue          = clearValues[attIndex];
					auto &image               = images[attIndex];
					auto &imageMemory         = imageMemories[attIndex];
					auto &imageView           = imageViews[attIndex];

					switch(type)
					{
						case AttType::FRAMEBUFFER:
							Attachment::setFramebufferAttachment(
								desc, clearValue,
								attIndex, format
							);
							break;
						case AttType::COLOR:
							Attachment::setColorAttachment(
								_swapchainExtent, _memProps, _logicalDevice, _physicalDevice,
								desc, clearValue, image, imageMemory, imageView,
								attIndex, format
							);
							break;
						case AttType::DEPTH:
							Attachment::setDepthAttachment(
								_swapchainExtent, _memProps, _logicalDevice, _physicalDevice,
								desc, clearValue, image, imageMemory, imageView,
								attIndex, format, !_isDefault ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_DEPTH_BIT
							);
							break;
						case AttType::INPUT:
							Attachment::setInputAttachment(
								_swapchainExtent, _memProps, _logicalDevice, _physicalDevice,
								desc, clearValue, image, imageMemory, imageView,
								attIndex, format
							);
						break;
					}

					attIndex++;
				}
			}

			template<uint16_t attCount>
			static VkFramebufferCreateInfo setFramebufferInfo(
				const VkRenderPass								&_renderPass,
				const VkExtent2D									&_extent,
				std::array<VkImageView, attCount> &_attachments
			) noexcept
			{
				VkFramebufferCreateInfo info	= {};
				info.sType										= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				info.pNext										= nullptr;
				info.renderPass								= _renderPass;
				info.attachmentCount					= _attachments.size();
				info.pAttachments							= _attachments.data();
				info.width										= _extent.width;
				info.height										= _extent.height;
				info.layers										= 1;

				return info;
			}

			static void create(
				const VkDevice &_logicalDevice,
				const VkFramebufferCreateInfo &_info,
				VkFramebuffer &_framebuffer
			) noexcept;
	};
}