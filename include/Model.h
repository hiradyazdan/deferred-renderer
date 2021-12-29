#pragma once

#include "AssetHelper.h"

class Model
{
	public:
		struct Data
		{
			std::vector<uint32_t>							indices;
			std::vector<AssetHelper::Vertex>	vertices;
			std::vector<vk::Mesh>							meshes;
		};

	public:
		void load(
			const std::string &_fileName
		) noexcept;
		void draw(
			const VkCommandBuffer		&_cmdBuffer,
			const VkPipeline				&_pipeline,
			const VkPipelineLayout	&_pipelineLayout
		) noexcept;

		inline Data &getData() noexcept { return m_data; }

	private:
		Data  m_data;
};