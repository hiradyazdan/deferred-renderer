#include "vk/Attachment.h"
#include "vk/Device.h"

namespace vk
{
	void Attachment::destroy(
		const VkDevice							&_logicalDevice,
		const VkImageView						&_imageView,
		const VkImage 							&_image,
		const VkDeviceMemory				&_imageMemory,
		const VkAllocationCallbacks	*_pAllocator
	) noexcept
	{
		Image	::destroyImageView	(_logicalDevice, _imageView,		_pAllocator);
		Image	::destroyImage			(_logicalDevice, _image,				_pAllocator);
		destroyAttMemory					(_logicalDevice, _imageMemory,	_pAllocator);
	}

	void Attachment::destroyAttMemory(
		const VkDevice							&_logicalDevice,
		const VkDeviceMemory				&_imageMemory,
		const VkAllocationCallbacks	*_pAllocator
	) noexcept
	{
		Device::freeMemory(_logicalDevice, _imageMemory,	_pAllocator);
	}
}