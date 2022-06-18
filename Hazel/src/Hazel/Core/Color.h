#pragma once

#include <glm/glm.hpp>

namespace Hazel
{
	class Color final
	{
	public:
		static const Color Black;	  // [0x000000] Color(0.0f, 0.0f, 0.0f, 1.0f);
		static const Color White;	  // [0xffffff] Color(1.0f, 1.0f, 1.0f, 1.0f);
		static const Color Clear;	  // [0x000000] Color(0.0f, 0.0f, 0.0f, 0.0f);
		static const Color Red;		  // [0xff0000] Color(1.0f, 0.0f, 0.0f, 1.0f);
		static const Color Green;	  // [0x00ff00] Color(0.0f, 1.0f, 0.0f, 1.0f);
		static const Color Blue;	  // [0x0000ff] Color(0.0f, 0.0f, 1.0f, 1.0f);
		static const Color Cyan;	  // [0x00ffff] Color(0.0f, 1.0f, 1.0f, 1.0f);
		static const Color Gray;	  // [0x7f7f7f] Color(0.5f, 0.5f, 0.5f, 1.0f);
		static const Color Grey;	  // [0x7f7f7f] Color(0.5f, 0.5f, 0.5f, 1.0f);
		static const Color Magenta;	  // [0xff00ff] Color(1.0f, 0.0f, 1.0f, 1.0f);
		static const Color Turquoise; // [0x40E0D0] Color(0.251f, 0.878f, 0.816f, 1.0f);
		static const Color Yellow;    // [0xffea04] Color(1.0f, 0.92f, 0.016f, 1.0f);

		/// <summary>
		/// Covert from Hue, Saturation, Value/Brightness to Red Green Blue Color with alpha
		/// </summary>
		/// <param name="h">Hue</param>
		/// <param name="s">Saturation</param>
		/// <param name="v">Value/Brightness</param>
		/// <param name="a">Alpha</param>
		static Color HSVtoRGB(float h, float s, float v, float a = 1.0f);

		/// <summary>
		/// Covert from vec4 to RGB Color
		/// <para> vec4.x = Hue				</para>
		/// <para> vec4.y = Saturation		</para>
		/// <para> vec4.z = Value/Brightness</para>
		/// <para> vec4.w = Alpha			</para>
		/// </summary>
		/// <param name="hsv">x(Hue), y(Saturation), z(Value/Brightness)</param>
		static Color HSVtoRGB(const glm::vec4& hsv);

		/// <summary>
		/// From Red Green Blue in extracts the Hue, Saturation, Value/Brightness
		/// </summary>
		/// <param name="color">RGBA Color</param>
		/// <param name="H">Hue</param>
		/// <param name="S">Saturation</param>
		/// <param name="V">Value/Brightness</param>
		static void RGBtoHSV(const Color& color, float& H, float& S, float& V);

		/// <summary>
		/// Convert from Red Green Blue Color to Hue, Saturation, Value/Brightness
		/// <para> vec4.x = Hue				</para>
		/// <para> vec4.y = Saturation		</para>
		/// <para> vec4.z = Value/Brightness</para>
		/// <para> vec4.w = Alpha			</para>
		/// </summary>
		/// <param name="color">RGBA Color</param>
		static glm::vec4 RGBtoHSV(const Color& color);

		/// <summary>
		/// From Red Green Blue in extracts the Hue, Saturation, Value/Brightness
		/// </summary>
		/// <param name="color">RGBA Color</param>
		/// <param name="H">Hue</param>
		/// <param name="S">Saturation</param>
		/// <param name="L">Lightness</param>
		static void RGBtoHSL(const Color& color, float& H, float& S, float& L);

		/// <summary>
		/// Convert from Red Green Blue Color to Hue, Saturation, Value/Brightness
		/// <para> vec4.x = Hue				</para>
		/// <para> vec4.y = Saturation		</para>
		/// <para> vec4.z = Lightness		</para>
		/// <para> vec4.w = Alpha			</para>
		/// </summary>
		/// <param name="color">RGB Color</param>
		static glm::vec4 RGBtoHSL(const Color& color);

		/// <summary>
		/// Convert from Red Green Blue Color to GrayScale version.
		/// </summary>
		/// <param name="color">RGB Color</param>
		static Color RGBtoGrayscale(const Color& color);

		/// <summary>
		/// Covert from hex to color
		/// <para> "FFFFFF" = White </para>
		/// </summary>
		/// <param name="hexValue">Hexadecimal value</param>
		static Color HEXtoRGB(const std::string& hexValue);

		/// <summary>
		/// Generate a random value for each components.
		/// </summary>
		static Color Random();

		/// <summary>
		/// Returns a color lerped from colorA to colorB
		/// </summary>
		/// <param name="colorA">from color</param>
		/// <param name="colorB">to color</param>
		/// <param name="lerpValue">from to percentage</param>
		static Color Lerp(const Color& colorA, const Color& colorB, float lerpValue);

		/// <summary>
		/// Returns a color lerped from colorA to colorB
		/// Values returned can be lower than 0.0f and higher than 1.0f
		/// </summary>
		/// <param name="colorA">from color</param>
		/// <param name="colorB">to color</param>
		/// <param name="lerpValue">from to percentage</param>
		static Color LerpUnclamped(const Color& colorA, const Color& colorB, float lerpValue);

	public:
		Color() = default;
		Color(const Color& color) = default;
		Color(float value);
		Color(glm::vec4 value);
		Color(int red, int green, int blue, float alpha = 1.0f);
		Color(float red, float green, float blue, float alpha = 1.0f);
		Color(double red, double green, double blue, double alpha = 1.0);
		~Color() = default;

		/// <summary>
		/// Return the Grayscale value of the color. (ITU-R BT.601 conversion)
		/// (0.299 * r) + (0.587 * g) + (0.114 * b)
		/// </summary>
		float GetGrayscaleValue() const;

		/// <summary>
		/// Return address of r component.
		/// </summary>
		float* GetValuePtr();

		/// <summary>
		/// Return hexadecimal version of the RGB
		/// </summary>
		std::string GetHexValue() const;

	public:
		float r = 0.0f; // Red
		float g = 0.0f; // Green
		float b = 0.0f; // Blue
		float a = 1.0f; // Alpha

	public:
		/// <summary>
		/// Explicitly convert to glm::vec4.
		/// </summary>
		operator glm::vec4() const;

		///// <summary>
		///// Returns r, g, b, a using [0], [1], [2], [3] respectively.
		///// </summary>
		float& operator[](int index);

		/// <summary>
		/// Returns r, g, b, a using [0], [1], [2], [3] respectively.
		/// </summary>
		float operator[](int index) const;

		/// <summary>
		/// Add each components separately.
		/// </summary>
		Color operator +(const Color& other) const;

		/// <summary>
		/// Subtract each components separately.
		/// </summary>
		Color operator -(const Color& other) const;

		/// <summary>
		/// Multiply each components separately.
		/// </summary>
		Color operator *(const Color& other) const;

		/// <summary>
		/// Divide each components by a value.
		/// </summary>
		Color operator /(float value) const;

		/// <summary>
		/// Add each components separately.
		/// </summary>
		Color& operator +=(const Color& other);

		/// <summary>
		/// Subtract each components separately.
		/// </summary>
		Color& operator -=(const Color& other);

		/// <summary>
		/// Multiply each components separately.
		/// </summary>
		Color& operator *=(const Color& other);

		/// <summary>
		/// Divide each components by a value.
		/// </summary>
		Color& operator /=(float value);

		/// <summary>
		/// Compare each components separately.
		/// </summary>
		bool operator ==(const Color& other) const;
	};
}

// Formating  color for log
template <>
struct fmt::formatter<Hazel::Color>
{
	// Presentation format: 'f' - fixed
	char presentation = 'f';

	constexpr auto parse(fmt::format_parse_context& ctx)
	{
		// Parse the presentation format and store it in the formatter:
		auto it = ctx.begin(), end = ctx.end();
		if (it != end && (*it == 'f')) presentation = *it++;

		// Check if reached the end of the range:
		if (it != end && *it != '}')
			throw format_error("invalid format");

		// Return an iterator past the end of the parsed range:
		return it;
	}

	template <typename FormatContext>
	auto format(const Hazel::Color& color, FormatContext& ctx)
	{
		return format_to(ctx.out(), "[0x{}](r:{:.3f}, g:{:.3f}, b:{:.3f}, a:{:.3f})", color.GetHexValue(), color.r, color.g, color.b, color.a);
	}
};
