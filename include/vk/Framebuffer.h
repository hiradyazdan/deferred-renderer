#pragma once

#include "Attachment.h"
#include "Swapchain.h"

namespace vk
{
	class Framebuffer
	{
		friend class RenderPass;
		friend class Command;

		using AttSubpassMap = Attachment::AttSubpassMap;

		public:
			template<uint16_t attCount>
			struct Data
			{
				Attachment::Data<attCount>	attachments;

				VkFramebuffer	framebuffer	= VK_NULL_HANDLE;
				VkRenderPass	renderPass	= VK_NULL_HANDLE;
			};

		public:
			inline static void create(
				const VkDevice								&_logicalDevice,
				const VkFramebufferCreateInfo	&_info,
				VkFramebuffer									&_framebuffer
			) noexcept
			{
				auto result = vkCreateFramebuffer(
					_logicalDevice,
					&_info,
					nullptr,
					&_framebuffer
				);
				ASSERT_VK(result, "Failed to create Framebuffer");
			}

		public:
			template<uint16_t attCount>
			inline static void createAttachments(
				const VkDevice													&_logicalDevice,
				const VkPhysicalDeviceMemoryProperties	&_memProps,
				const Array<AttSubpassMap, attCount>		&_attSpMaps,
				Attachment::Data<attCount>							&_attachmentsData,
				bool																		_isDefault = false
			) noexcept
			{
				using AttType = Attachment::Type;

				auto &descs         = _attachmentsData.descs;
				auto &formats				= _attachmentsData.formats;
				auto &clearValues   = _attachmentsData.clearValues;

				auto &images        = _attachmentsData.images;
				auto &imageMemories = _attachmentsData.imageMemories;
				auto &imageViews    = _attachmentsData.imageViews;
				auto &extent				= _attachmentsData.extent;

				uint32_t attIndex = 0;
				for(const auto &attSpMap : _attSpMaps)
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
								extent, _memProps, _logicalDevice,
								desc, clearValue, image, imageMemory, imageView,
								format
							);
							break;
						case AttType::DEPTH:
						{
							_attachmentsData.depthAttIndex = attIndex;
							Attachment::setDepthAttachment(
								extent, _memProps, _logicalDevice,
								desc, clearValue, image, imageMemory, imageView,
								format,
								format >= vk::FormatType::D16_UNORM_S8_UINT
								? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT
								: VK_IMAGE_ASPECT_DEPTH_BIT,
								_isDefault
								? image::UsageFlag::DEPTH_STENCIL_ATTACHMENT
								: image::UsageFlag::DEPTH_STENCIL_ATTACHMENT | VK_IMAGE_USAGE_SAMPLED_BIT
							);
						}
							break;
						case AttType::INPUT:
							Attachment::setInputAttachment(
								extent, _memProps, _logicalDevice,
								desc, clearValue, image, imageMemory, imageView,
								format
							);
						break;
					}

					attIndex++;
				}
			}

			template<size_t attCount>
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

			template<uint16_t attCount>
			static void recreate(
				const VkDevice													&_logicalDevice,
				const VkPhysicalDeviceMemoryProperties	&_memProps,
				const std::vector<VkImageView>					&_scImageViews,
				const VkRenderPass											&_renderPass,
				Attachment::Data<attCount>							&_attachmentData,
				std::vector<VkFramebuffer>							&_scFramebuffers
			) noexcept
			{
				using AttType = Attachment::Type;

				auto &attData					= _attachmentData;
				const auto DEPTH			= attData.depthAttIndex;

				auto &depthView				= attData.imageViews		[DEPTH];
				auto &depthImage			= attData.images				[DEPTH];
				auto &depthMemory			= attData.imageMemories	[DEPTH];

				auto &extent					= attData.extent;
				auto &depthDesc				= attData.descs					[DEPTH];
				auto &depthClearValue	= attData.clearValues		[DEPTH];
				auto &depthFormat			= attData.formats				[DEPTH];

				Attachment::destroy(
					_logicalDevice,
					depthView,
					depthImage,
					depthMemory
				);

				const auto &aspectMask = depthFormat >= vk::FormatType::D16_UNORM_S8_UINT
					? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT
					: VK_IMAGE_ASPECT_DEPTH_BIT;

				Attachment::setDepthAttachment(
					extent, _memProps, _logicalDevice,
					depthDesc, depthClearValue, depthImage, depthMemory, depthView,
					depthFormat, aspectMask
				);

				destroy(_logicalDevice, _scFramebuffers);

				Array<VkImageView, attCount> attachments = {};
				attachments[AttType::DEPTH] = depthView;

				auto fbInfo = setFramebufferInfo(
					_renderPass,
					extent,
					attachments
				);

				auto fbIndex = 0;
				for(auto &framebuffer : _scFramebuffers)
				{
					attachments[AttType::FRAMEBUFFER] = _scImageViews[fbIndex];

					create(_logicalDevice, fbInfo, framebuffer);

					fbIndex++;
				}
			}

			inline static void destroy(
				const VkDevice							&_logicalDevice,
				const VkFramebuffer					&_framebuffer,
				const VkAllocationCallbacks *_pAllocator = nullptr
			) noexcept
			{
				vkDestroyFramebuffer(
					_logicalDevice,
					_framebuffer,
					_pAllocator
				);
			}

			inline static void destroy(
				const VkDevice							&_logicalDevice,
				std::vector<VkFramebuffer>	&_framebuffers,
				const VkAllocationCallbacks *_pAllocator = nullptr
			) noexcept
			{
				for(auto &framebuffer : _framebuffers)
				{
					destroy(
						_logicalDevice,
						framebuffer,
						_pAllocator
					);
				}
			}
	};
}