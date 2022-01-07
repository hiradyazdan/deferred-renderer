#include "vk/Texture.h"

namespace vk
{
	bool Texture::isKtx(const std::string &_uri) noexcept
	{
		const auto &extPos = _uri.find_last_of('.');

		if(extPos != std::string::npos)
		{
			if(_uri.substr(extPos + 1) == "ktx")
			{
				return true;
			}
		}

		return false;
	}

	void Texture::load(
		const std::string		&_fileName,
		const VkFormat			&_format,
		const VkDevice			&_logicalDevice, const VkQueue				&_copyQueue,
		const VkImageUsageFlags &_imageUsageFlags,
		const VkImageLayout	&_imageLayout,	bool _isLinearTiling
	) noexcept
	{
		ktxTexture *ktxTexture;

		auto result = load(_fileName, &ktxTexture);

		ASSERT(result == KTX_SUCCESS, "Failed to load ktx texture!");

		auto width = ktxTexture->baseWidth;
		auto height = ktxTexture->baseHeight;
		auto mipLevels = ktxTexture->numLevels;

		auto ktxTextureData = ktxTexture_GetData(ktxTexture);
		auto ktxTextureSize = ktxTexture->dataSize;
		auto ktxTextureFormat = ktxTexture_GetVkFormat(ktxTexture);


	}

	void Texture::load() noexcept
	{

	}

	ktxResult Texture::load(
		const std::string	&_fileName,
		ktxTexture				**_texture
	) noexcept
	{
		ktxResult result = KTX_SUCCESS;

#if defined(__ANDROID__)
		AAsset* asset = AAssetManager_open(androidApp->activity->assetManager, filename.c_str(), AASSET_MODE_STREAMING);
		if (!asset) {
			vks::tools::exitFatal("Could not load texture from " + filename + "\n\nMake sure the assets submodule has been checked out and is up-to-date.", -1);
		}
		size_t size = AAsset_getLength(asset);
		assert(size > 0);
		ktx_uint8_t *textureData = new ktx_uint8_t[size];
		AAsset_read(asset, textureData, size);
		AAsset_close(asset);
		result = ktxTexture_CreateFromMemory(textureData, size, KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, target);
		delete[] textureData;
#else
		// @todo Should verify if the file exists
		result = ktxTexture_CreateFromNamedFile(
			_fileName.c_str(),
			KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT,
			_texture
		);
#endif

		return result;
	}
}