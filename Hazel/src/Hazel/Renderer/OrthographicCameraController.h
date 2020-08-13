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

	private:
		bool OnMouseScrolled(MouseScrolledEvent& event);
		bool OnWindowResized(WindowResizeEvent& event);

		void Reset();

	public:
		OrthographicCamera& GetCamera() { return _camera; }
		const OrthographicCamera& GetCamera() const { return _camera; }

	private:
		float _aspectRation;
		float _zoomLevel = 1.0f;
		OrthographicCamera _camera;

		bool _rotation;

		glm::vec3 _cameraPosition = { 0.0f,0.0f,0.0f };
		float _cameraRotation = 0.0f;
		float _cameraTranslationSpeed = 5.0f;
		float _cameraRotationSpeed = 180.0f;
	};
}
