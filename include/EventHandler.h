#pragma once

template<typename TRenderer>
class EventHandler
{
	public:
		inline static EventHandler *getInstance(GLFWwindow *_window) noexcept
		{
			s_instance = s_instance == nullptr
				? new EventHandler(_window)
				: s_instance;

			return s_instance;
		}

		~EventHandler() = default;

	public:
		inline void setEventCallbacks() noexcept
		{
			setKeyCallback();
			setMouseBtnCallback();
			setMouseScrollCallback();
			setMouseMoveCallback();
		}

	private:
		explicit EventHandler(GLFWwindow *_window) : m_window(_window) {}

	private:
		inline void setKeyCallback() noexcept
		{
			glfwSetKeyCallback(m_window, [](
				GLFWwindow *_window,
				int _key, int _scanCode, int _action, int _mods
			)
			{
				auto &camera = static_cast<TRenderer*>(glfwGetWindowUserPointer(_window))->getCamera();

				switch (_action)
				{
					case GLFW_PRESS:		setKey(camera, _key, true);		break;
					case GLFW_RELEASE:	setKey(camera, _key, false);	break;
					default: break;
				}
			});
		}
		inline void setMouseBtnCallback() noexcept
		{
			glfwSetMouseButtonCallback(m_window, [](
				GLFWwindow *_window,
				int _btn, int _action, int _mods
			)
			{
				auto &camera = static_cast<TRenderer*>(glfwGetWindowUserPointer(_window))->getCamera();

				glm::vec<2, double> mousePos {};

				glfwGetCursorPos(_window, &mousePos.x, &mousePos.y);

				switch (_action)
				{
					case GLFW_PRESS:		setKey(camera, _btn, true, mousePos);		break;
					case GLFW_RELEASE:	setKey(camera, _btn, false, mousePos);	break;
					default: break;
				}
			});
		}
		inline void setMouseScrollCallback() noexcept
		{
			glfwSetScrollCallback(m_window, [](
				GLFWwindow *_window,
				double _xOffset, double _yOffset
			)
			{
				auto *renderer = static_cast<TRenderer*>(glfwGetWindowUserPointer(_window));
				Camera &camera = renderer->getCamera();

				camera.updateByScroll(_xOffset, _yOffset);
			});
		}
		inline void setMouseMoveCallback() noexcept
		{
			glfwSetCursorPosCallback(m_window, [](
				GLFWwindow *_window,
				double _xPos, double _yPos
			)
			{
				Camera &camera = static_cast<TRenderer*>(glfwGetWindowUserPointer(_window))->getCamera();

				camera.updateByBtn(_xPos, _yPos);
			});
		}

	private:
		inline static void setKey(
			const Camera &_camera, int _key, bool _isPressed,
			glm::vec2 _mousePos = {}
		) noexcept
		{
			using CamData = Camera::Data;
			using Keys 		= CamData::Keys;
			using Btns		= CamData::MouseBtns;

			if(_camera.getData().type != Camera::Data::Type::FIRST_PERSON) return;

			switch (_key)
			{
				case GLFW_KEY_W:	_camera.setKey<Keys::UP>		(_isPressed);	break;
				case GLFW_KEY_S:	_camera.setKey<Keys::DOWN>	(_isPressed);	break;
				case GLFW_KEY_A:	_camera.setKey<Keys::LEFT>	(_isPressed);	break;
				case GLFW_KEY_D:	_camera.setKey<Keys::RIGHT>	(_isPressed);	break;

				case GLFW_KEY_UP:			_camera.setKey<Keys::UP>		(_isPressed);	break;
				case GLFW_KEY_DOWN:		_camera.setKey<Keys::DOWN>	(_isPressed);	break;
				case GLFW_KEY_LEFT:		_camera.setKey<Keys::LEFT>	(_isPressed);	break;
				case GLFW_KEY_RIGHT:	_camera.setKey<Keys::RIGHT>	(_isPressed);	break;

				case GLFW_MOUSE_BUTTON_LEFT:		_camera.setBtn<Btns::LEFT>	(_mousePos, _isPressed); break;
				case GLFW_MOUSE_BUTTON_RIGHT:		_camera.setBtn<Btns::RIGHT>	(_mousePos, _isPressed); break;
				case GLFW_MOUSE_BUTTON_MIDDLE:	_camera.setBtn<Btns::MIDDLE>(_mousePos, _isPressed); break;

				default: break;
			}
		}

	private:
		GLFWwindow *m_window;
		inline static EventHandler *s_instance = nullptr;
};