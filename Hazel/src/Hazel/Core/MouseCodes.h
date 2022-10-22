#pragma once

namespace Hazel
{
	using Mouse = enum class MouseCode : uint16_t
	{
		Button1 = 0,
		Button2 = 1,
		Button3 = 2,
		Button4 = 3,
		Button5 = 4,
		Button6 = 5,
		Button7 = 6,
		Button8 = 7,

		COUNT,

		ButtonLast = Button8,
		ButtonLeft = Button1,
		ButtonRight = Button2,
		ButtonMiddle = Button3
	};

	inline std::ostream& operator<<(std::ostream& os, const MouseCode& button)
	{
		os << static_cast<int32_t>(button);
		return os;
	}
}

#pragma once

// From glfw3.h
#define HZ_MOUSE_BUTTON_1      ::Hazel::Mouse::Button1
#define HZ_MOUSE_BUTTON_2      ::Hazel::Mouse::Button2
#define HZ_MOUSE_BUTTON_3      ::Hazel::Mouse::Button3
#define HZ_MOUSE_BUTTON_4      ::Hazel::Mouse::Button4
#define HZ_MOUSE_BUTTON_5      ::Hazel::Mouse::Button5
#define HZ_MOUSE_BUTTON_6      ::Hazel::Mouse::Button6
#define HZ_MOUSE_BUTTON_7      ::Hazel::Mouse::Button7
#define HZ_MOUSE_BUTTON_8      ::Hazel::Mouse::Button8

#define HZ_MOUSE_BUTTON_LAST   ::Hazel::Mouse::ButtonLast
#define HZ_MOUSE_BUTTON_LEFT   ::Hazel::Mouse::ButtonLeft
#define HZ_MOUSE_BUTTON_RIGHT  ::Hazel::Mouse::ButtonRight
#define HZ_MOUSE_BUTTON_MIDDLE ::Hazel::Mouse::ButtonMiddle
