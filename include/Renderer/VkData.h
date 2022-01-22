#pragma once

#include "vk/vk.h"
#include "../_constants.h"

// GPU / Shader Data

enum class vk::Attachment::Tag::Color : uint16_t
{
	POSITION	= 0,
	NORMAL		= 1,
	ALBEDO		= 2,
	_count_ = 3
};

enum class vk::Descriptor::LayoutCategory : uint16_t
{
	DEFERRED_SHADING = 0,
	_count_ = 1
};

enum class vk::Pipeline::Type : uint16_t
{
	COMPOSITION = 0, // lighting pass (deferred)
	OFFSCREEN		= 1, // geometry pass (g-buffer)
	_count_ = 2
};

enum class vk::Buffer::Category : uint16_t
{
	COMPOSITION = 0, // lighting pass (deferred)
	OFFSCREEN		= 1, // geometry pass (g-buffer)
	_count_ = 2
};

enum class vk::Texture::Sampler : uint16_t
{
	COLOR		= 0,
	NORMAL	= 1,
	_count_	= 2
};

enum class vk::Shader::Stage : uint16_t
{
	VERTEX		= 0,
	FRAGMENT	= 1,
	_count_ = 2
};

enum class vk::Model::ID : uint16_t
{
	SPONZA	= 0,
	_count_ = 1
};

inline const uint16_t vk::Attachment::s_attCount				= 2;
inline const uint16_t vk::RenderPass::s_subpassCount		= 1;
inline const uint16_t vk::RenderPass::s_spDepCount			= 2;

inline const uint16_t vk::Buffer		::s_ubcCount				= vk::toInt(vk::Buffer		::Category			::_count_);
inline const uint16_t vk::Descriptor::s_setLayoutCount	= vk::toInt(vk::Descriptor::LayoutCategory::_count_);
inline const uint16_t vk::Model			::s_modelCount			= vk::toInt(vk::Model			::ID						::_count_);
inline const uint16_t vk::Buffer		::s_bufferCount			= vk::Buffer::s_mbtCount + vk::Buffer::s_ubcCount;
inline const uint16_t vk::Pipeline	::s_layoutCount			= 1;
inline const uint16_t vk::Pipeline	::s_pushConstCount	= 1;

static_assert(
	vk::Model::s_modelCount == constants::models.size(),
	"Model filenames and IDs should have an equal count."
);