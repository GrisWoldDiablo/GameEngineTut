#include "hzpch.h"
#include "Color.h"
#include "Random.h"

namespace Hazel
{
	const Color Color::Black = Color(0.0f, 0.0f, 0.0f, 1.0f);
	const Color Color::White = Color(1.0f, 1.0f, 1.0f, 1.0f);
	const Color Color::Clear = Color(0.0f, 0.0f, 0.0f, 0.0f);
	const Color Color::Red = Color(1.0f, 0.0f, 0.0f, 1.0f);
	const Color Color::Green = Color(0.0f, 1.0f, 0.0f, 1.0f);
	const Color Color::Blue = Color(0.0f, 0.0f, 1.0f, 1.0f);
	const Color Color::Cyan = Color(0.0f, 1.0f, 1.0f, 1.0f);
	const Color Color::Gray = Color(0.5f, 0.5f, 0.5f, 1.0f);
	const Color Color::Grey = Color(0.5f, 0.5f, 0.5f, 1.0f);
	const Color Color::Magenta = Color(1.0f, 0.0f, 1.0f, 1.0f);
	const Color Color::Yellow = Color(1.0f, 0.92f, 0.016f, 1.0f);

	Color Color::HSVtoRGB(float h, float s, float v, float a)
	{
		return  HSVtoRGB({ h, s, v, a});
	}

	Color Color::HSVtoRGB(const glm::vec4& hsv)
	{
		double H = hsv.x;
		double S = hsv.y / 100.0;
		double V = hsv.z / 100.0;

		double C = S * V;
		auto Hprime = H / 60.0;
		double X = C * (1 - abs(fmod(Hprime, 2) - 1));
		double m = V - C;
		double R, G, B;

		if (Hprime >= 0 && Hprime < 1)
		{
			R = C;
			G = X;
			B = 0;
		}
		else if (Hprime >= 1 && Hprime < 2)
		{
			R = X;
			G = C;
			B = 0;
		}
		else if (Hprime >= 2 && Hprime < 3)
		{
			R = 0;
			G = C;
			B = X;
		}
		else if (Hprime >= 3 && Hprime < 4)
		{
			R = 0;
			G = X;
			B = C;
		}
		else if (Hprime >= 4 && Hprime < 5)
		{
			R = X;
			G = 0;
			B = C;
		}
		else
		{
			R = C;
			G = 0;
			B = X;
		}

		return Color((float)(R + m), (float)(G + m), (float)(B + m), hsv.a);
	}

	void Color::RGBtoHSV(const Color& color, float& H, float& S, float& V)
	{
		auto hsv = RGBtoHSV(color);
		H = hsv.r;
		S = hsv.g;
		V = hsv.b;
	}

	glm::vec4 Color::RGBtoHSV(const Color& color)
	{
		double H = 0;
		double S = 0;
		double V = 0;
		double r = color.r;
		double g = color.g;
		double b = color.b;

		double Xmax = glm::max(glm::max(color.r, color.g), color.b);
		double Xmin = glm::min(glm::min(color.r, color.g), color.b);
		double C = Xmax - Xmin;

		if (C == 0)
		{
			H = 0;
		}
		else if (Xmax == r)
		{
			H = fmod((60 * ((g - b) / C) + 360), 360.0);
		}
		else if (Xmax == g)
		{
			H = fmod((60 * ((b - r) / C) + 120), 360.0);
		}
		else if (Xmax == b)
		{
			H = fmod((60 * ((r - g) / C) + 240), 360.0);
		}

		if (Xmax == 0)
		{
			S = 0;
		}
		else
		{
			S = (C / Xmax) * 100.0;
		}

		V = Xmax * 100.0;

		return { H, S, V , color.a };
	}

	Color Color::Random()
	{
		return Color(Hazel::Random::Float(), Hazel::Random::Float(), Hazel::Random::Float(), Hazel::Random::Float());
	}

	Color Color::Lerp(const Color& colorA, const Color& colorB, float lerpValue)
	{
		auto color = LerpUnclamped(colorA, colorB, lerpValue);
		color.r = glm::clamp(color.r, 0.0f, 1.0f);
		color.g = glm::clamp(color.g, 0.0f, 1.0f);
		color.b = glm::clamp(color.b, 0.0f, 1.0f);
		color.a = glm::clamp(color.a, 0.0f, 1.0f);
		return color;
	}

	Color Color::LerpUnclamped(const Color& colorA, const Color& colorB, float lerpValue)
	{
		auto red = (colorB.r * lerpValue) + (colorA.r * (1.0f - lerpValue));
		auto green = (colorB.g * lerpValue) + (colorA.g * (1.0f - lerpValue));
		auto blue = (colorB.b * lerpValue) + (colorA.b * (1.0f - lerpValue));
		auto alpha = (colorB.a * lerpValue) + (colorA.a * (1.0f - lerpValue));
		return Color(red, green, blue, alpha);
	}

	Color::Color(float value)
		:Color(value, value, value)
	{
	}

	Color::Color(glm::vec4 value)
		:Color(value.r, value.g, value.b, value.a)
	{
	}

	Color::Color(int red, int green, int blue, float alpha)
		:Color(red / 255.0f, green / 255.0f, blue / 255.0f, alpha)
	{
	}

	Color::Color(float red, float green, float blue, float alpha)
		:r(red), g(green), b(blue), a(alpha)
	{
	}

	float* Color::GetValuePtr()
	{
		return &r;
	}

	Color::operator glm::vec4() const
	{
		return { r,g,b,a };
	}

	float& Color::operator[](int index)
	{
		switch (index)
		{
		case 0:
			return r;
			break;
		case 1:
			return g;
			break;
		case 2:
			return b;
			break;
		case 3:
			return a;
			break;
		}

		HZ_CORE_ASSERT(0, "Index [" + std::to_string(index) + "] out of range!");
		throw nullptr;
	}

	Color Color::operator+(const Color other)
	{
		auto red = glm::clamp(this->r + other.r, 0.0f, 1.0f);
		auto green = glm::clamp(this->g + other.g, 0.0f, 1.0f);
		auto blue = glm::clamp(this->b + other.b, 0.0f, 1.0f);
		auto alpha = glm::clamp(this->a + other.a, 0.0f, 1.0f);
		return Color(red, green, blue, alpha);
	}

	Color Color::operator-(const Color other)
	{
		auto red = glm::clamp(this->r - other.r, 0.0f, 1.0f);
		auto green = glm::clamp(this->g - other.g, 0.0f, 1.0f);
		auto blue = glm::clamp(this->b - other.b, 0.0f, 1.0f);
		auto alpha = glm::clamp(this->a - other.a, 0.0f, 1.0f);
		return Color(red, green, blue, alpha);
	}

	Color Color::operator*(const Color other)
	{
		auto red = this->r * other.r;
		auto green = this->g * other.g;
		auto blue = this->b * other.b;
		auto alpha = this->a * other.a;
		return Color(red, green, blue, alpha);
	}

	Color Color::operator/(float value)
	{
		HZ_CORE_ASSERT(value, "Cannot divive by Zero!");
		auto red = this->r / value;
		auto green = this->g / value;
		auto blue = this->b / value;
		auto alpha = this->a / value;
		return Color(red, green, blue, alpha);
	}

	Color& Color::operator+=(const Color other)
	{
		this->r = glm::clamp(this->r + other.r, 0.0f, 1.0f);
		this->g = glm::clamp(this->g + other.g, 0.0f, 1.0f);
		this->b = glm::clamp(this->b + other.b, 0.0f, 1.0f);
		this->a = glm::clamp(this->a + other.a, 0.0f, 1.0f);
		return *this;
	}

	Color& Color::operator-=(const Color other)
	{
		this->r = glm::clamp(this->r - other.r, 0.0f, 1.0f);
		this->g = glm::clamp(this->g - other.g, 0.0f, 1.0f);
		this->b = glm::clamp(this->b - other.b, 0.0f, 1.0f);
		this->a = glm::clamp(this->a - other.a, 0.0f, 1.0f);
		return *this;
	}

	Color& Color::operator*=(const Color other)
	{
		this->r *= other.r;
		this->g *= other.g;
		this->b *= other.b;
		this->a *= other.a;
		return *this;
	}

	Color& Color::operator/=(float value)
	{
		HZ_CORE_ASSERT(value, "Cannot divive by Zero!");
		this->r /= value;
		this->g /= value;
		this->b /= value;
		this->a /= value;
		return *this;
	}

	bool Color::operator==(const Color other)
	{
		return (this->r == other.r && this->g == other.g &&
			this->b == other.b && this->a == other.a);
	}
}
