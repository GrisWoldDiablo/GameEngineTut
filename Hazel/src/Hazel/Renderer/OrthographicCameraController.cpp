#include "hzpch.h"
#include "OrthographicCameraController.h"

#include "Hazel/Core/Input.h"
#include "Hazel/Core/KeyCodes.h"

namespace Hazel
{

	OrthographicCameraController::OrthographicCameraController(float aspectRation, bool rotation)
		:_aspectRation(aspectRation), _camera(-_aspectRation * _zoomLevel, _aspectRation* _zoomLevel, -_zoomLevel, _zoomLevel), _rotation(rotation)
	{
	}

	void OrthographicCameraController::OnUpdate(Timestep timestep)
	{
		HZ_PROFILE_FUNCTION();
		
		if (Input::IsKeyPressed(HZ_KEY_W))
		{
			_cameraPosition.x += -sin(glm::radians(_cameraRotation)) * _cameraTranslationSpeed * timestep;
			_cameraPosition.y += cos(glm::radians(_cameraRotation)) * _cameraTranslationSpeed * timestep;
		}
		else if (Input::IsKeyPressed(HZ_KEY_S))
		{
			_cameraPosition.x -= -sin(glm::radians(_cameraRotation)) * _cameraTranslationSpeed * timestep;
			_cameraPosition.y -= cos(glm::radians(_cameraRotation)) * _cameraTranslationSpeed * timestep;
		}

		if (Input::IsKeyPressed(HZ_KEY_A))
		{
			_cameraPosition.x -= cos(glm::radians(_cameraRotation)) * _cameraTranslationSpeed * timestep;
			_cameraPosition.y -= sin(glm::radians(_cameraRotation)) * _cameraTranslationSpeed * timestep;
		}
		else if (Input::IsKeyPressed(HZ_KEY_D))
		{
			_cameraPosition.x += cos(glm::radians(_cameraRotation)) * _cameraTranslationSpeed * timestep;
			_cameraPosition.y += sin(glm::radians(_cameraRotation)) * _cameraTranslationSpeed * timestep;
		}

		if (_rotation)
		{
			if (Input::IsKeyPressed(HZ_KEY_Q))
			{
				_cameraRotation += _cameraRotationSpeed * timestep;
			}
			else if (Input::IsKeyPressed(HZ_KEY_E))
			{
				_cameraRotation -= _cameraRotationSpeed * timestep;
			}
			
			if (_cameraRotation > 180.0f)
			{
				_cameraRotation -= 360.0f;
			}
			else if (_cameraRotation <= -180.0f)
			{
				_cameraRotation += 360.0f;
			}

			_camera.SetRotation(_cameraRotation);
		}

		if (Input::IsKeyPressed(HZ_KEY_R))
		{
			Reset();
		}

		_camera.SetPosition(_cameraPosition);
	}

	void OrthographicCameraController::OnEvent(Event& event)
	{
		HZ_PROFILE_FUNCTION();
		
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<MouseScrolledEvent>(HZ_BIND_EVENT_FN(OnMouseScrolled));
		dispatcher.Dispatch<WindowResizeEvent>(HZ_BIND_EVENT_FN(OnWindowResized));
	}

	void OrthographicCameraController::SetZoomLevel(float level)
	{
		HZ_PROFILE_FUNCTION();
		
		_zoomLevel = level;
		_zoomLevel = std::max(_zoomLevel, _zoomLevelMinimum);
		_camera.SetProjection(-_aspectRation * _zoomLevel, _aspectRation * _zoomLevel, -_zoomLevel, _zoomLevel);

		// Slow camera as you zoom in and speed it up as you zoom out
		_cameraTranslationSpeed = _zoomLevel;
	}

	void OrthographicCameraController::SetPosition(glm::vec3 value)
	{
		_cameraPosition = value;
		_camera.SetPosition(_cameraPosition);
	}

	void OrthographicCameraController::SetRotation(float value)
	{
		_cameraRotation = value;
		_camera.SetRotation(_cameraRotation);
	}

	void OrthographicCameraController::SetDefaults()
	{
		_defaultZoomLevel = GetZoomLevel();
		_defaultRotation = GetRotation();
		_defaultPosition = GetPosition();
	}

	bool OrthographicCameraController::OnMouseScrolled(MouseScrolledEvent& event)
	{
		HZ_PROFILE_FUNCTION();
		
		SetZoomLevel(_zoomLevel - event.GetYOffset() * _zoomLevelSpeed);
		return false;
	}

	bool OrthographicCameraController::OnWindowResized(WindowResizeEvent& event)
	{
		HZ_PROFILE_FUNCTION();
		
		_aspectRation = (float)event.GetWidth() / (float)event.GetHeight();
		_camera.SetProjection(-_aspectRation * _zoomLevel, _aspectRation * _zoomLevel, -_zoomLevel, _zoomLevel);
		return false;
	}

	void OrthographicCameraController::Reset()
	{
		HZ_PROFILE_FUNCTION();

		SetZoomLevel(_defaultZoomLevel);
		SetRotation(_defaultRotation);
		SetPosition(_defaultPosition);
	}
}
