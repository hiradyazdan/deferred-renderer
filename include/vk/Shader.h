#pragma once

#include "utils.h"

namespace vk
{
	class Shader
	{
		public:
			enum class Stage : uint16_t;

			template<uint16_t stageCount>
			struct Data
			{
				using StageFlag = shader::StageFlag;

				Data() { ASSERT_ENUMS(Stage); }

				struct Temp
				{
					STACK_ONLY(Temp);

					const Array<VkShaderStageFlagBits, 12> stageFlags = {
						StageFlag::VERTEX,
						StageFlag::FRAGMENT,
						StageFlag::COMPUTE,
						StageFlag::GEOMETRY,
						StageFlag::TESSELLATION_CONTROL,
						StageFlag::TESSELLATION_EVALUATION,
						StageFlag::RAYGEN_KHR,
						StageFlag::ANY_HIT_KHR,
						StageFlag::CLOSEST_HIT_KHR,
						StageFlag::MISS_KHR,
						StageFlag::INTERSECTION_KHR,
						StageFlag::CALLABLE_KHR
					};
				};

				Array<VkPipelineShaderStageCreateInfo, stageCount>	stages;

				VkShaderModule	module			= VK_NULL_HANDLE;
				uint16_t				moduleIndex	= 0;

				bool isValid() const noexcept { return module != VK_NULL_HANDLE; }
			};

		public:
			template<Stage stage, uint16_t stageCount>
			inline static VkPipelineShaderStageCreateInfo load(
				const VkDevice				&_logicalDevice,
				const std::string			&_fileName,
				VkShaderModule				&_module,
				VkSpecializationInfo	*_specializationInfo = nullptr
			) noexcept
			{
				VkPipelineShaderStageCreateInfo stageInfo;

				load(_logicalDevice, _fileName, _module);

				const auto tempData = Data<stageCount>::Temp::create();

				stageInfo.sType								= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				stageInfo.pNext								= nullptr;
				stageInfo.flags								= 0;
				stageInfo.stage								= tempData.stageFlags[stage];
				stageInfo.module							= _module;
				stageInfo.pName								= "main";
				stageInfo.pSpecializationInfo	= _specializationInfo;

				return stageInfo;
			}

			inline static VkSpecializationMapEntry setSpecializationMapEntry(
				uint32_t _constantId,
				uint32_t _offset,
				size_t _size
			) noexcept
			{
				VkSpecializationMapEntry mapEntry = {};

				mapEntry.constantID	= _constantId;
				mapEntry.offset			= _offset;
				mapEntry.size				= _size;

				return mapEntry;
			}

			inline static VkSpecializationInfo setSpecializationInfo(
				const void											*_data,
				size_t													_dataSize,
				const VkSpecializationMapEntry	*_pMapEntries,
				uint32_t												_mapEntryCount
			) noexcept
			{
				VkSpecializationInfo specializationInfo = {};

				specializationInfo.mapEntryCount	= _mapEntryCount;
				specializationInfo.pMapEntries		= _pMapEntries;
				specializationInfo.dataSize				= _dataSize;
				specializationInfo.pData					= _data;

				return specializationInfo;
			}

		private:
			inline static void load(
				const VkDevice		&_logicalDevice,
				const std::string	&_fileName,
				VkShaderModule		&_module
			) noexcept
			{
				const auto &filePath = _fileName + ".spv";

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
						&_module
					);
					ASSERT_VK(result, "Failed to create Shader Module");

					delete[] rawSPVBytes;
				}
				else
				{
					FATAL_ERROR_LOG("Could not open shader file @ %s", filePath.c_str());
				}
			}
	};
}