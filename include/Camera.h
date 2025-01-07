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

			glm::vec3 rot			= glm::vec3();
			glm::vec3 pos			= glm::vec3();
			glm::vec4 viewPos	= glm::vec4();

			float rotSpeed	= 1.0f;
			float moveSpeed	= 1.0f;

			struct
			{
				glm::mat4 perspective;
				glm::mat4 view;
			} matrices;

			enum class Keys : uint16_t
			{
				LEFT	= 0,
				RIGHT	= 1,
				UP		= 2,
				DOWN	= 3
			};
			enum class MouseBtns : uint16_t
			{
				LEFT		= 0,
				RIGHT		= 1,
				MIDDLE	= 2
			};

			private:
				mutable vk::Array<bool, 4> m_keys		= { false, false, false, false };
				mutable vk::Array<bool, 4> m_mBtns	= { false, false, false, false };
				mutable glm::vec2 m_mousePos;

				bool isYFlipped	= false;
				bool isUpdated	= false;

			private:
				float m_fov;
				float m_zNear;
				float m_zFar;
		};

	public:
		inline Data getData()		const noexcept { return m_data; }
		inline bool isUpdated() const noexcept { return m_data.isUpdated; }
		inline bool isMoving(bool _isKeyOnly = false)	const noexcept
		{
			using Keys = Data::Keys;
			using Btns = Data::MouseBtns;

			auto &keys = m_data.m_keys;
			auto &btns = m_data.m_mBtns;

			auto isKeyPressed = keys[Keys::LEFT] || keys[Keys::RIGHT] || keys[Keys::UP] || keys[Keys::DOWN];
			auto isBtnPressed = btns[Btns::LEFT] || btns[Btns::RIGHT] || btns[Btns::MIDDLE];

			return _isKeyOnly ? isKeyPressed : isKeyPressed || isBtnPressed;
		}
		template<Data::Keys key>
		inline void setKey(bool _isPressed) const noexcept { m_data.m_keys[key]		= _isPressed; }
		template<Data::MouseBtns btn>
		inline void setBtn(const glm::vec2 &_pos, bool _isPressed) const noexcept
		{
			m_data.m_mousePos = _isPressed ? _pos : m_data.m_mousePos;
			m_data.m_mBtns[btn]	= _isPressed;
		}
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
		inline void translate(glm::vec3 _delta)
		{
			m_data.pos += _delta;
			updateViewMatrix();
		}
		inline void rotate(glm::vec3 _delta)
		{
			m_data.rot += _delta;
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
			m_data.isYFlipped = !m_data.isYFlipped;
		}
		inline void updateAspectRatio(float _width, float _height) noexcept
		{
			if(_width <= 0.0f || _height <= 0.0f) return;

			setPerspective(_width / _height);
		}
		inline void updateByKey(float _deltaTime) noexcept
		{
			using Keys = Data::Keys;

			m_data.isUpdated = false;

			if(m_data.type == Data::Type::FIRST_PERSON)
			{
				if(isMoving(true))
				{
					glm::vec3 front;
					auto &rot = m_data.rot;
					auto &pos = m_data.pos;
					auto &keys = m_data.m_keys;

					front.x = -cos(glm::radians(rot.x)) * sin(glm::radians(rot.y));
					front.y = sin(glm::radians(rot.x));
					front.z = cos(glm::radians(rot.x)) * cos(glm::radians(rot.y));

					front = glm::normalize(front);

					auto moveSpeed = _deltaTime * m_data.moveSpeed;
					auto hMove = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;
					auto vMove = front * moveSpeed;

					if(keys[Keys::LEFT])	{ pos -= hMove; }
					if(keys[Keys::RIGHT])	{ pos += hMove; }
					if(keys[Keys::UP])		{ pos += vMove; }
					if(keys[Keys::DOWN])	{ pos -= vMove; }
				}
			}

			updateViewMatrix();
		}
		inline void updateByBtn(double _x, double _y) noexcept
		{
			using Btns = Data::MouseBtns;

			auto &rotSpeed = m_data.rotSpeed;
			auto &mousePos = m_data.m_mousePos;
			auto &btns = m_data.m_mBtns;
			auto dX = (int) mousePos.x - (int) _x;
			auto dY = (int) mousePos.y - (int) _y;

			if(btns[Btns::LEFT])		{ rotate(glm::vec3((float) dY * rotSpeed, -(float) dX * rotSpeed, 0.f)); }
			if(btns[Btns::RIGHT])		{ translate(glm::vec3(-0.f, 0.f, (float) dY * .005f)); }
			if(btns[Btns::MIDDLE])	{ translate(glm::vec3(-(float) dX * 0.005f, -(float) dY * 0.005f, 0.f)); }

			mousePos = glm::vec2((float) _x, (float) _y);
		}
		inline void updateByScroll(double _xOffset, double _yOffset) noexcept
		{
			translate(glm::vec3((float) _yOffset, 0.f, 0.f));
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

			if(m_data.isYFlipped)
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

			if(m_data.isYFlipped)
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

			m_data.viewPos = glm::vec4(m_data.pos, 0.0f) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);

			m_data.isUpdated = true;
		}

	private:
		Data m_data;
};