#include "vk/Shader.h"

namespace vk
{
	void Shader::load(
		const VkDevice	&_logicalDevice,
		const char			*_fileName,
		Data 						&_data
	) noexcept
	{
		const auto &filePath = constants::SHADERS_PATH + _fileName + ".spv";

		std::ifstream stream(filePath, std::ios::binary | std::ios::in | std::ios::ate);

		if(stream.is_open())
		{
			auto size = stream.tellg();
			stream.seekg(0, std::ios::beg);

			auto rawSPVBytes = new char[size];
			stream.read(rawSPVBytes, size);
			stream.close();

			VkShaderModuleCreateInfo info = {};
			info.sType		= VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			info.codeSize	= size;
			info.pCode		= reinterpret_cast<const uint32_t*>(rawSPVBytes);

			const auto &result = vkCreateShaderModule(
				_logicalDevice,
				&info,
				nullptr,
				&_data.module
			);
			ASSERT_VK(result, "Failed to create Shader Module");

			delete[] rawSPVBytes;
		}
		else
		{
			FATAL_ERROR_LOG("Could not open shader file @ %s%", filePath.c_str());
		}
	}
}