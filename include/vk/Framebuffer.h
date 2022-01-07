#pragma once

#include "Attachment.h"
#include "Swapchain.h"

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
				const VkDevice													&_logicalDevice,
				const VkPhysicalDevice									&_physicalDevice,
				const VkPhysicalDeviceMemoryProperties	&_memProps,
				Attachment::Data<attCount>							&_attachmentsData,
				bool																		_isDefault = true
			) noexcept
			{
				auto &attSpMaps     = _attachmentsData.attSpMaps;
				auto &descs         = _attachmentsData.descs;
				auto &formats				= _attachmentsData.formats;
				auto &clearValues   = _attachmentsData.clearValues;

				auto &images        = _attachmentsData.images;
				auto &imageMemories = _attachmentsData.imageMemories;
				auto &imageViews    = _attachmentsData.imageViews;
				auto &extent				= _attachmentsData.extent;

				using AttType = typename Attachment::Data<attCount>::Type;

				uint32_t attIndex = 0;
				for(const auto &attSpMap : attSpMaps)
				{
					auto &type								= attSpMap.attType;
					auto &desc                = descs					[attIndex];
					auto &format							= formats				[attIndex];
					auto &clearValue          = clearValues		[attIndex];
					auto &image               = images				[attIndex];
					auto &imageMemory         = imageMemories	[attIndex];
					auto &imageView           = imageViews		[attIndex];

					switch(type)
					{
						case AttType::FRAMEBUFFER:
							Attachment::setFramebufferAttachment(
								desc, clearValue,
								format
							);
							break;
						case AttType::COLOR:
							Attachment::setColorAttachment(
								extent, _memProps, _logicalDevice, _physicalDevice,
								desc, clearValue, image, imageMemory, imageView,
								format
							);
							break;
						case AttType::DEPTH:
							_attachmentsData.depthAttIndex = attIndex;
							Attachment::setDepthAttachment(
								extent, _memProps, _logicalDevice, _physicalDevice,
								desc, clearValue, image, imageMemory, imageView,
								format, !_isDefault
								? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT
								: VK_IMAGE_ASPECT_DEPTH_BIT
							);
							break;
						case AttType::INPUT:
							Attachment::setInputAttachment(
								extent, _memProps, _logicalDevice, _physicalDevice,
								desc, clearValue, image, imageMemory, imageView,
								format
							);
						break;
					}

					attIndex++;
				}
			}

			template<uint16_t attCount>
			static VkFramebufferCreateInfo setFramebufferInfo(
				const VkRenderPass						&_renderPass,
				const VkExtent2D							&_extent,
				Array<VkImageView, attCount>	&_imageViews
			) noexcept
			{
				VkFramebufferCreateInfo info	= {};
				info.sType										= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				info.pNext										= nullptr;
				info.renderPass								= _renderPass;
				info.attachmentCount					= _imageViews.size();
				info.pAttachments							= _imageViews.data();
				info.width										= _extent.width;
				info.height										= _extent.height;
				info.layers										= 1;

				return info;
			}

			static void create(
				const VkDevice								&_logicalDevice,
				const VkFramebufferCreateInfo	&_info,
				VkFramebuffer									&_framebuffer
			) noexcept;

			template<uint16_t attCount>
			static void recreate(
				const VkDevice															&_logicalDevice,
				const VkPhysicalDevice											&_physicalDevice,
				const VkPhysicalDeviceMemoryProperties			&_memProps,
				const std::vector<Swapchain::Data::Buffer>	&_scBuffers,
				std::vector<VkFramebuffer>									&_scFramebuffers,
				vk::Framebuffer::Data<attCount>							&_fbData,
				bool																				_isDefault = true
			) noexcept
			{
				auto &attData			= _fbData.attachments;
				const auto DEPTH	= attData.depthAttIndex;

				auto &imageView		= attData.imageViews		[DEPTH];
				auto &image				= attData.images				[DEPTH];
				auto &imageMemory	= attData.imageMemories	[DEPTH];

				auto &extent			= attData.extent;
				auto &desc 				= attData.descs					[DEPTH];
				auto &clearValue	= attData.clearValues		[DEPTH];
				auto &format			= attData.formats				[DEPTH];

				Attachment::destroyAttachment(
					_logicalDevice,
					imageView,
					image,
					imageMemory
				);

				for(auto &framebuffer : _scFramebuffers)
				{
					vkDestroyFramebuffer(
						_logicalDevice,
						framebuffer,
						nullptr
					);
				}

				Attachment::setDepthAttachment(
					extent, _memProps, _logicalDevice, _physicalDevice,
					desc, clearValue, image, imageMemory, imageView,
					format,!_isDefault
					? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT
					: VK_IMAGE_ASPECT_DEPTH_BIT
				);

				auto fbInfo = setFramebufferInfo<attCount>(
					_fbData.renderPass,
					extent,
					attData.imageViews
				);

				auto attIndex = 0;
				for(auto &framebuffer : _scFramebuffers)
				{
					attData.imageViews[0] = _scBuffers[attIndex].imageView;

					create(_logicalDevice, fbInfo, framebuffer);

					attIndex++;
				}
			}
	};
}