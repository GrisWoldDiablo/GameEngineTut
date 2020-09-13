#pragma once
#include "Core.h"

namespace Hazel
{
	class Input
	{
	public:
		static bool IsKeyPressed(int keycode) { return _sInstance->IsKeyPressedImpl(keycode); }

		static bool IsMouseButtonPressed(int button) { return _sInstance->IsMouseButtonPressedImpl(button); }
		static std::pair<float,float> GetMousePosition() { return _sInstance->GetMousePositionImpl(); }
		static float GetMouseX() { return _sInstance->GetMouseXImpl(); }
		static float GetMouseY() { return _sInstance->GetMouseXImpl(); }

	protected:
		virtual bool IsKeyPressedImpl(int keycode) = 0;
		virtual bool IsMouseButtonPressedImpl(int button) = 0;
		virtual std::pair<float, float> GetMousePositionImpl() = 0;
		virtual float GetMouseXImpl() = 0;
		virtual float GetMouseYImpl() = 0;

	private:
		static Scope<Input> _sInstance;
	};
}