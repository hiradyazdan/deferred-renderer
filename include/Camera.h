#pragma once

#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "vk/vk.h"
#include "_constants.h"

class Camera
{
	public:
		struct Data
		{
			friend class Camera;

			enum class Type
			{
				LOOK_AT,
				FIRST_PERSON
			};
			Type type = Type::LOOK_AT;

			glm::vec3 rot = glm::vec3();
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
		inline const Data getData() const noexcept { return m_data; }
		inline void setType(const Data::Type &_type) noexcept
		{
			m_data.type = _type;
		}
		inline void setPosition(const glm::vec3 &_pos) noexcept
		{
			m_data.pos = _pos;
			updateViewMatrix();
		}
		inline void setRotation(const glm::vec3 &_rot) noexcept
		{
			m_data.rot = _rot;
			updateViewMatrix();
		}
		inline void setPerspective(
			float _fov, float _aspect, float _zNear, float _zFar
		) noexcept
		{
			m_data.m_fov		= _fov;
			m_data.m_zNear	= _zNear;
			m_data.m_zFar		= _zFar;

			setPerspective(_aspect);
		}
		inline void flipY() noexcept
		{
			m_data.flipY = !m_data.flipY;
		}
		inline void updateAspectRatio(float _width, float _height) noexcept
		{
			if(_width <= 0.0f || _height <= 0.0f) return;

			setPerspective(_width / _height);
		}

	private:
		inline void setPerspective(float _aspect) noexcept
		{
			auto &perspective = m_data.matrices.perspective;

			perspective = glm::perspective(
				glm::radians(m_data.m_fov),
				_aspect,
				m_data.m_zNear, m_data.m_zFar
			);

			if(m_data.flipY)
			{
				perspective[1][1] *= -1.0f;
			}
		}
		inline void updateViewMatrix()
		{
			glm::mat4 identityMtx = glm::mat4(1.0f);
			glm::mat4 rotMtx;
			glm::mat4 transMtx;

			auto translation 	= m_data.pos;
			auto rotation 		= m_data.rot;

			if(m_data.flipY)
			{
				rotation.x		*= -1.0f;
				translation.y	*= -1.0f;
			}

			rotMtx = glm::rotate(identityMtx,	glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
			rotMtx = glm::rotate(rotMtx,			glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
			rotMtx = glm::rotate(rotMtx,			glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

			transMtx = glm::translate(identityMtx, translation);

			m_data.matrices.view = m_data.type == Data::Type::FIRST_PERSON
				? rotMtx * transMtx
				: transMtx * rotMtx;

			m_data.isUpdated = true;
		}

	private:
		Data m_data;
};