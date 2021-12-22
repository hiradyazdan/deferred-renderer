#pragma once

#include "AssetHelper.h"

class Model
{
	public:
		void load(
			const std::string &_fileName
		) noexcept;
		void draw() noexcept;
};