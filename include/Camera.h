#pragma once

#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "macros.h"

class Camera
{
	public:
		struct Data
		{
			glm::vec3 pos = glm::vec3();

			struct
			{
				glm::mat4 perspective;
				glm::mat4 view;
			} matrices;
		};

	public:
		inline Data &getData() noexcept { return m_data; }

	private:
		Data m_data;
};