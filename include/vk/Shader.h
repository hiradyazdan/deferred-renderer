#pragma once

#include "utils.h"

namespace vk
{
	class Shader
	{
		public:
			enum class Stage : uint16_t;

			struct Data
			{
				const VkShaderStageFlagBits stageFlags[12] = {
					shader::StageFlag::VERTEX,
					shader::StageFlag::FRAGMENT,
					shader::StageFlag::COMPUTE,
					shader::StageFlag::GEOMETRY,
					shader::StageFlag::TESSELLATION_CONTROL,
					shader::StageFlag::TESSELLATION_EVALUATION,
					shader::StageFlag::RAYGEN_KHR,
					shader::StageFlag::ANY_HIT_KHR,
					shader::StageFlag::CLOSEST_HIT_KHR,
					shader::StageFlag::MISS_KHR,
					shader::StageFlag::INTERSECTION_KHR,
					shader::StageFlag::CALLABLE_KHR
				};

				VkPipelineShaderStageCreateInfo stageInfo = {};
				VkShaderModule module = VK_NULL_HANDLE;

				bool isValid() const noexcept { return module != VK_NULL_HANDLE; }
			};

		public:
			template<Stage stage>
			static void load(
				const VkDevice	&_logicalDevice,
				const char			*_fileName,
				Data 						&_data
			) noexcept
			{
				load(_logicalDevice, _fileName, _data);
				auto &stageInfo = _data.stageInfo;

				stageInfo.sType								= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				stageInfo.pNext								= nullptr;
				stageInfo.flags								= 0;
				stageInfo.stage								= getStageFlag<stage>(_data.stageFlags);
				stageInfo.module							= _data.module;
				stageInfo.pName								= "main";
				stageInfo.pSpecializationInfo	= nullptr;
			}

		private:
			static void load(
				const VkDevice	&_logicalDevice,
				const char			*_fileName,
				Data 						&_data
			) noexcept;

			template<Stage stage>
			static VkShaderStageFlagBits getStageFlag(const VkShaderStageFlagBits *_stageFlags) noexcept
			{
				return _stageFlags[static_cast<int>(stage)];
			}
	};
}