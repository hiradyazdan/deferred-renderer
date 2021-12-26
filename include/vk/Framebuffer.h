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
				auto &types         = _attachmentsData.types;
				auto &refs          = _attachmentsData.refsTemp;
				auto &descs         = _attachmentsData.descs;
				auto &formats				= _attachmentsData.formats;
				auto &clearValues   = _attachmentsData.clearValues;

				auto &images        = _attachmentsData.images;
				auto &imageMemories = _attachmentsData.imageMemories;
				auto &imageViews    = _attachmentsData.imageViews;

				refs.resize(attCount);

				using AttType = typename Attachment::Data<attCount>::Type;

				uint32_t index = 0;
				for(const auto &type : types)
				{
					auto &ref                 = refs[index];
					auto &desc                = descs[index];
					auto &format							= formats[index];
					auto &clearValue          = clearValues[index];
					auto &image               = images[index];
					auto &imageMemory         = imageMemories[index];
					auto &imageView           = imageViews[index];

					types[index] = type;

					switch(type)
					{
						case AttType::FRAMEBUFFER:
							Attachment::setFramebufferAttachment(
								ref, desc, clearValue,
								index, format
							);
						case AttType::COLOR:
							Attachment::setColorAttachment(
								_swapchainExtent, _memProps, _logicalDevice, _physicalDevice,
								ref, desc, clearValue, image, imageMemory, imageView,
								index, format
							);
							break;
						case AttType::DEPTH:
							Attachment::setDepthAttachment(
								_swapchainExtent, _memProps, _logicalDevice, _physicalDevice,
								ref, desc, clearValue, image, imageMemory, imageView,
								index, format, !_isDefault ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_DEPTH_BIT
							);
							break;
						case AttType::INPUT:
							Attachment::setInputAttachment(
								_swapchainExtent, _memProps, _logicalDevice, _physicalDevice,
								ref, desc, clearValue, image, imageMemory, imageView,
								index, format
							);
						break;
					}

					index++;
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