#pragma once
#include "Base.h"
#include "Hazel/Core/KeyCodes.h"
#include "Hazel/Core/MouseCodes.h"

namespace Hazel
{
	class Input
	{
	protected:
		Input() = default;
	public:
		virtual ~Input() = default;
		
		Input(const Input&) = delete;
		Input& operator=(const Input&) = delete;

		static bool IsKeyPressed(KeyCode key) { return _sInstance->IsKeyPressedImpl(key); }

		static bool IsMouseButtonPressed(MouseCode button) { return _sInstance->IsMouseButtonPressedImpl(button); }
		static std::pair<float,float> GetMousePosition() { return _sInstance->GetMousePositionImpl(); }
		static float GetMouseX() { return _sInstance->GetMouseXImpl(); }
		static float GetMouseY() { return _sInstance->GetMouseXImpl(); }

	protected:
		virtual bool IsKeyPressedImpl(KeyCode key) = 0;

		virtual bool IsMouseButtonPressedImpl(MouseCode button) = 0;
		virtual std::pair<float, float> GetMousePositionImpl() = 0;
		virtual float GetMouseXImpl() = 0;
		virtual float GetMouseYImpl() = 0;

	private:
		static Scope<Input> _sInstance;
	};
}