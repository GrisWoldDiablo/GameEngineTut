#include "hzpch.h"
#include "OrthographicCameraController.h"

#include "Hazel/Input.h"
#include "Hazel/KeyCodes.h"

namespace Hazel
{

	OrthographicCameraController::OrthographicCameraController(float aspectRation, bool rotation)
		:_aspectRation(aspectRation), _camera(-_aspectRation * _zoomLevel, _aspectRation* _zoomLevel, -_zoomLevel, _zoomLevel), _rotation(rotation)
	{

	}

	void OrthographicCameraController::OnUpdate(Timestep timestep)
	{
		if (Input::IsKeyPressed(HZ_KEY_W))
		{
			_cameraPosition.y += _cameraTranslationSpeed * timestep;
		}
		else if (Input::IsKeyPressed(HZ_KEY_S))
		{
			_cameraPosition.y -= _cameraTranslationSpeed * timestep;
		}

		if (Input::IsKeyPressed(HZ_KEY_A))
		{
			_cameraPosition.x -= _cameraTranslationSpeed * timestep;
		}
		else if (Input::IsKeyPressed(HZ_KEY_D))
		{
			_cameraPosition.x += _cameraTranslationSpeed * timestep;
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
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<MouseScrolledEvent>(HZ_BIND_EVENT_FN(OnMouseScrolled));
		dispatcher.Dispatch<WindowResizeEvent>(HZ_BIND_EVENT_FN(OnWindowResized));
	}

	bool OrthographicCameraController::OnMouseScrolled(MouseScrolledEvent& event)
	{
		_zoomLevel -= event.GetYOffset() * 0.25f;
		_zoomLevel = std::max(_zoomLevel, 0.1f);
		_camera.SetProjection(-_aspectRation * _zoomLevel, _aspectRation * _zoomLevel, -_zoomLevel, _zoomLevel);

		// Slow camera as you zoom in and speed it up as you zoom out
		_cameraTranslationSpeed = _zoomLevel;

		return false;
	}

	bool OrthographicCameraController::OnWindowResized(WindowResizeEvent& event)
	{
		_aspectRation = (float)event.GetWidth() / (float)event.GetHeight();
		_camera.SetProjection(-_aspectRation * _zoomLevel, _aspectRation * _zoomLevel, -_zoomLevel, _zoomLevel);
		return false;
	}

	void OrthographicCameraController::Reset()
	{
		_zoomLevel = 1.0f;
		_camera.SetProjection(-_aspectRation * _zoomLevel, _aspectRation * _zoomLevel, -_zoomLevel, _zoomLevel);

		_cameraRotation = 0.0f;
		_camera.SetRotation(_cameraRotation);
		
		_cameraPosition = glm::vec3(0.0f);
		_camera.SetPosition(_cameraPosition);
	}
}
