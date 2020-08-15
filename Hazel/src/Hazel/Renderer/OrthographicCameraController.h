#pragma once
#include "OrthographicCamera.h"
#include "Hazel/Core/Timestep.h"

#include "Hazel/Events/ApplicationEvent.h"
#include "Hazel/Events/MouseEvent.h"

namespace Hazel
{
	class OrthographicCameraController
	{
	public:
		OrthographicCameraController(float aspectRation, bool rotation = false);
		~OrthographicCameraController() = default;

		void OnUpdate(Timestep timestep);
		void OnEvent(Event& event);

		OrthographicCamera& GetCamera() { return _camera; }
		const OrthographicCamera& GetCamera() const { return _camera; }

		float GetZoomLevel() { return _zoomLevel; }
		void SetZoomLevel(float value);
		float GetZoomLevelSpeed() { return _zoomLevelSpeed; }
		void SetZoomLevelSpeed(float value) { _zoomLevelSpeed = value; }
		float GetZoomLevelMinimum() { return _zoomLevelMinimum; }
		void SetZoomLevelMinimum(float value) { _zoomLevelMinimum = value; }
		glm::vec3 GetPosition() { return _cameraPosition; }
		void SetPosition(glm::vec3 value)
		{
			_cameraPosition = value;
			_camera.SetPosition(_cameraPosition);
		}
		float GetRotation() { return _cameraRotation; }
		void SetRotation(float value)
		{
			_cameraRotation = value;
			_camera.SetRotation(_cameraRotation);
		}

	private:
		bool OnMouseScrolled(MouseScrolledEvent& event);
		bool OnWindowResized(WindowResizeEvent& event);

		void Reset();

	private:
		float _aspectRation;
		float _zoomLevel = 1.0f;
		OrthographicCamera _camera;
		bool _rotation;

		float _zoomLevelSpeed = 0.25f;
		float _zoomLevelMinimum = 0.1f;

		glm::vec3 _cameraPosition = { 0.0f,0.0f,0.0f };
		float _cameraRotation = 0.0f;
		float _cameraTranslationSpeed = 5.0f;
		float _cameraRotationSpeed = 180.0f;
	};
}