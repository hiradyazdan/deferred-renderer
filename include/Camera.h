#pragma once

#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "macros.h"

class Camera
{
	public:
		struct Data
		{
			friend class Camera;

			glm::vec3 pos = glm::vec3();

			bool flipY = false;

			struct
			{
				glm::mat4 perspective;
				glm::mat4 view;
			} matrices;

			bool isUpdated	= false;

			private:
				float m_fov;
				float m_zNear;
				float m_zFar;
		};

	public:
		inline Data &getData() noexcept { return m_data; }
		inline void setPerspective(
			float _fov, float _aspect, float _zNear, float _zFar
		) noexcept
		{
			m_data.m_fov		= _fov;
			m_data.m_zNear	= _zNear;
			m_data.m_zFar		= _zFar;

			setPerspective(_aspect);
		}
		inline void updateAspectRatio(float _width, float _height) noexcept
		{
			if(_width <= 0.0f || _height <= 0.0f) return;

			setPerspective(_width / _height);
		}

	private:
		inline void setPerspective(float _aspect) noexcept
		{
			m_data.matrices.perspective = glm::perspective(
				glm::radians(m_data.m_fov),
				_aspect,
				m_data.m_zNear, m_data.m_zFar
			);

			if(m_data.flipY)
			{
				m_data.matrices.perspective[1][1] *= -1.0f;
			}
		}

	private:
		Data m_data;
};