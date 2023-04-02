#pragma once

#include "Hazel/Core/KeyCodes.h"
#include "Hazel/Core/MouseCodes.h"

namespace Hazel
{
	class Input
	{
		enum class Status
		{
			None = 0,
			Pressed,
			Down,
			Up
		};

	public:
		static bool IsValidKeyCodeValue(int keyCodeValue)
		{
			return keyCodeValue >= 0 && keyCodeValue < static_cast<int>(KeyCode::LAST);
		}

		static bool IsKeyPressed(KeyCode keyCode)
		{
			return Get()._keysStatus[GetKeyIndex(keyCode)] == Status::Pressed;
		}

		static bool IsKeyDown(KeyCode keyCode)
		{
			return Get()._keysStatus[GetKeyIndex(keyCode)] == Status::Down;
		}

		static bool IsKeyUp(KeyCode keyCode)
		{
			return Get()._keysStatus[GetKeyIndex(keyCode)] == Status::Up;
		}

		static bool IsValidMouseCodeValue(int mouseCodeValue)
		{
			return mouseCodeValue >= 0 && mouseCodeValue < static_cast<int>(MouseCode::COUNT);
		}

		static bool IsMouseButtonPressed(MouseCode mouseCode)
		{
			return Get()._buttonsStatus[static_cast<uint16_t>(mouseCode)] == Status::Pressed;
		}

		static bool IsMouseButtonDown(MouseCode mouseCode)
		{
			return Get()._buttonsStatus[static_cast<uint16_t>(mouseCode)] == Status::Down;
		}

		static bool IsMouseButtonUp(MouseCode mouseCode)
		{
			return Get()._buttonsStatus[static_cast<uint16_t>(mouseCode)] == Status::Up;
		}

		static glm::vec2 GetMousePosition();
		static float GetMouseX();
		static float GetMouseY();

	private:
		Status& GetKeyStatus(KeyCode keyCode) { return _keysStatus[GetKeyIndex(keyCode)]; }
		Status& GetMouseStatus(MouseCode mouseCode) { return _buttonsStatus[static_cast<uint16_t>(mouseCode)]; }

		void UpdateDownStatus();
		void UpdateUpStatus();

		std::array<Status, static_cast<uint16_t>(KeyCode::COUNT)> _keysStatus;
		std::array<Status, static_cast<uint16_t>(MouseCode::COUNT)> _buttonsStatus;

		static Input& Get();

		static int GetKeyIndex(KeyCode keyCode)
		{
			return static_cast<uint16_t>(keyCode) - static_cast<uint16_t>(KeyCode::FIRST);
		}

		friend class Application;
		friend class WindowsWindow;
	};
}
