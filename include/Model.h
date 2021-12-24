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

			vk::Pipeline::Data								pipelineData;
		};

	public:
		void load(
			const std::string &_fileName
		) noexcept;
		void draw(
			const VkCommandBuffer &_cmdBuffer
		) noexcept;

		inline Data &getData() noexcept { return m_data; }

	private:
		Data  m_data;
};