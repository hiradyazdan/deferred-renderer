#pragma once

#include "utils.h"
#include "Framebuffer.h"

namespace vk
{
	class RenderPass
	{
		public:
			template<uint16_t framebufferAttCount, uint16_t subpassCount, uint16_t subpassDepCount>
			struct Data
			{
				struct Subpass
				{
					std::vector<VkAttachmentReference> colorRefs;
					std::vector<VkAttachmentReference> depthRefs;
					std::vector<VkAttachmentReference> inputRefs;

					VkSubpassDescription desc = {};
				};

				Framebuffer::Data<framebufferAttCount> framebufferData;

				std::array<Subpass*,						subpassCount>			subpasses;
				std::array<VkSubpassDependency,	subpassDepCount>	deps;
			};

		public:
			template<uint16_t attCount, uint16_t subpassCount, uint16_t subpassDepCount>
			static void create(
				const VkDevice																																										&_logicalDevice,
				const std::array<VkAttachmentDescription, attCount>																								&_attDescs,
				const std::array<typename Data<attCount, subpassCount, subpassDepCount>::Subpass*, subpassCount>	&_subpasses,
				const std::array<VkSubpassDependency,			subpassDepCount>																				&_subpassDeps,
				VkRenderPass																																											&_renderPass
			) noexcept
			{
				std::array<VkSubpassDescription, subpassCount> subpasses;

				for(auto i = 0u; i < subpassCount; i++)
				{
					subpasses[i] = _subpasses[i]->desc;
				}

				VkRenderPassCreateInfo info = {};

				info.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

				info.attachmentCount = _attDescs.size();
				info.pAttachments    = _attDescs.data();

				info.subpassCount    = subpasses.size();
				info.pSubpasses      = subpasses.data();

				info.dependencyCount = _subpassDeps.size();
				info.pDependencies   = _subpassDeps.data();

				const auto &result = vkCreateRenderPass(
					_logicalDevice,
					&info,
					nullptr,
					&_renderPass
				);
				ASSERT_VK(result, "Failed to create render pass!");

				for(auto i = 0u; i < subpassCount; i++) { delete _subpasses[i]; }
			}

			template<uint16_t attCount, uint16_t subpassCount, uint16_t subpassDepCount>
			static void createSubpasses(
				const std::array<typename Attachment::Data<attCount>::AttSubpassMap, attCount>							&_attSpMaps,
				std::array<typename Data<attCount, subpassCount, subpassDepCount>::Subpass*, subpassCount>	&_subpasses
			) noexcept
			{
				using Subpass = typename Data<attCount, subpassCount, subpassDepCount>::Subpass;
				using AttType = typename Attachment::Data<attCount>::Type;

				// TODO: fix the logic! should not repeat subpass creation for each attachment map

				uint32_t attIndex = 0;
				for(const auto &attSpMap : _attSpMaps)
				{
					auto &subpassIndices = attSpMap.spIndices;

					ASSERT(
						subpassIndices.size() <= subpassCount,
						"Subpass indices size should not exceed the total number of subpasses!"
					);

					for(auto subpassIndex : subpassIndices)
					{
						ASSERT(
							subpassIndex < subpassCount,
							"Subpass index should be less than total number of subpasses!"
						);

						_subpasses[subpassIndex] = new Subpass();

						auto &subpass			= _subpasses[subpassIndex];
						auto &subpassDesc	= subpass->desc;

						auto &colorRefs		= subpass->colorRefs;
						auto &depthRefs		= subpass->depthRefs;
						auto &inputRefs		= subpass->inputRefs;

						switch(attSpMap.attType)
						{
							case AttType::FRAMEBUFFER:
							case AttType::COLOR:
								colorRefs.push_back({ attIndex, image::LayoutType::COLOR_ATTACHMENT_OPTIMAL });
								break;
							case AttType::DEPTH:
								depthRefs.push_back({ attIndex, image::LayoutType::DEPTH_STENCIL_ATTACHMENT_OPTIMAL });
								break;
							case AttType::INPUT:
								inputRefs.push_back({ attIndex, image::LayoutType::SHADER_READ_ONLY_OPTIMAL });
								break;
						}

						subpassDesc.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;

						subpassDesc.colorAttachmentCount    = colorRefs.size();
						subpassDesc.pColorAttachments       = colorRefs.data();

						subpassDesc.pDepthStencilAttachment	= depthRefs.data();

						subpassDesc.inputAttachmentCount    = inputRefs.size();
						subpassDesc.pInputAttachments       = inputRefs.data();

						// TODO
						subpassDesc.preserveAttachmentCount	= 0;
						subpassDesc.pPreserveAttachments		= nullptr;
						subpassDesc.pResolveAttachments			= nullptr;
					}

					attIndex++;
				}
			}
	};
}