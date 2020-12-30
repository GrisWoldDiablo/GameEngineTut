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
	const Color Color::Turquoise = Color(0.251f, 0.878f, 0.816f, 1.0f);
	const Color Color::Yellow = Color(1.0f, 0.92f, 0.016f, 1.0f);

	Color Color::HSVtoRGB(float h, float s, float v, float a)
	{
		return  HSVtoRGB({ h, s, v, a });
	}

	Color Color::HSVtoRGB(const glm::vec4& hsv)
	{
		HZ_PROFILE_FUNCTION();

		double H = hsv.x;
		double S = hsv.y;
		double V = hsv.z;

		double C = S * V;
		double Hprime = H * 6.0;
		double X = C * (1 - abs(fmod(Hprime, 2) - 1));
		double m = V - C;
		double R, G, B;

		auto floorHprime = (int)glm::floor(Hprime);
		switch (floorHprime)
		{
		case 0:
		{
			R = C;
			G = X;
			B = 0;
			break;
		}
		case 1:
		{
			R = X;
			G = C;
			B = 0;
			break;
		}
		case 2:
		{
			R = 0;
			G = C;
			B = X;
			break;
		}
		case 3:
		{
			R = 0;
			G = X;
			B = C;
			break;
		}
		case 4:
		{
			R = X;
			G = 0;
			B = C;
			break;
		}
		default:
		{
			R = C;
			G = 0;
			B = X;
			break;
		}
		}
		return Color((float)(R + m), (float)(G + m), (float)(B + m), hsv.a);
	}

	void Color::RGBtoHSV(const Color& color, float& H, float& S, float& V)
	{
		auto hsv = RGBtoHSV(color);
		H = hsv.x;
		S = hsv.y;
		V = hsv.z;
	}

	glm::vec4 Color::RGBtoHSV(const Color& color)
	{
		HZ_PROFILE_FUNCTION();

		double H = 0;
		double S = 0;
		double V = 0;
		double r = color.r;
		double g = color.g;
		double b = color.b;

		double Xmax = glm::max(glm::max(r, g), b);
		double Xmin = glm::min(glm::min(r, g), b);
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

		H /= 360.0;

		if (Xmax == 0)
		{
			S = 0;
		}
		else
		{
			S = (C / Xmax);
		}

		V = Xmax;

		return { H, S, V , color.a };
	}

	void Color::RGBtoHSL(const Color& color, float& H, float& S, float& L)
	{
		auto hsl = RGBtoHSL(color);
		H = hsl.x;
		S = hsl.y;
		L = hsl.z;
	}

	glm::vec4 Color::RGBtoHSL(const Color& color)
	{
		HZ_PROFILE_FUNCTION();

		double H = 0;
		double S = 0;
		double L = 0;
		double r = color.r;
		double g = color.g;
		double b = color.b;

		double Xmax = glm::max(glm::max(r, g), b);
		double Xmin = glm::min(glm::min(r, g), b);
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

		H /= 360.0;

		if (Xmax == 0)
		{
			S = 0;
		}
		else
		{
			S = (C / Xmax);
		}

		L = (Xmax - Xmin) / 2.0f;

		return { H, S, L , color.a };
	}

	Color Color::RGBtoGrayscale(const Color& color)
	{
		HZ_PROFILE_FUNCTION();

		auto grayscaleValue = color.GetGrayscaleValue();
		return  Color(grayscaleValue, grayscaleValue, grayscaleValue, 1.0f);
	}

	Color Color::HEXtoRGB(const std::string& hexValue)
	{
		HZ_PROFILE_FUNCTION();

		HZ_CORE_ASSERT(hexValue.length() == 6, "RGB Hexadecimal value need to consist of 6 characters.");

		for (auto character : hexValue)
		{
			switch (toupper(character))
			{
			case 0: case 1: case 2: case 3: case 4:
			case 5: case 6: case 7: case 8:case 9:
			case 'A': case 'B': case 'C':
			case 'D': case 'E': case 'F':
				break;
			default:
				HZ_CORE_ASSERT(false, std::string("[] is not a valid Hexadecimal character.").insert(1, 1, character));
				break;
			}
		}

		int red;
		int green;
		int blue;

		std::stringstream stream;

		stream << std::hex << hexValue.substr(0, 2);
		stream >> red;

		stream.clear();
		stream << std::hex << hexValue.substr(2, 2);
		stream >> green;

		stream.clear();
		stream << std::hex << hexValue.substr(4, 2);
		stream >> blue;

		return Color(red, green, blue);
	}

	Color Color::Random()
	{
		HZ_PROFILE_FUNCTION();

		return Color(Hazel::Random::Float(), Hazel::Random::Float(), Hazel::Random::Float(), Hazel::Random::Float());
	}

	Color Color::Lerp(const Color& colorA, const Color& colorB, float lerpValue)
	{
		HZ_PROFILE_FUNCTION();

		auto color = LerpUnclamped(colorA, colorB, lerpValue);
		color.r = glm::clamp(color.r, 0.0f, 1.0f);
		color.g = glm::clamp(color.g, 0.0f, 1.0f);
		color.b = glm::clamp(color.b, 0.0f, 1.0f);
		color.a = glm::clamp(color.a, 0.0f, 1.0f);
		return color;
	}

	Color Color::LerpUnclamped(const Color& colorA, const Color& colorB, float lerpValue)
	{
		HZ_PROFILE_FUNCTION();

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
		: Color(value.r, value.g, value.b, value.a)
	{
	}

	Color::Color(int red, int green, int blue, float alpha)
		: Color(red / 255.0f, green / 255.0f, blue / 255.0f, alpha)
	{
	}

	Color::Color(float red, float green, float blue, float alpha)
		: r(red), g(green), b(blue), a(alpha)
	{
	}

	float Color::GetGrayscaleValue() const
	{
		HZ_PROFILE_FUNCTION();

		return (0.299f * r) + (0.587f * g) + (0.114f * b);
	}

	float* Color::GetValuePtr()
	{
		return &r;
	}

	std::string Color::GetHexValue() const
	{
		std::stringstream stream;
		stream << std::hex << std::uppercase
			<< std::setfill('0') << std::setw(2) << (int)(r * 255)
			<< std::setfill('0') << std::setw(2) << (int)(g * 255)
			<< std::setfill('0') << std::setw(2) << (int)(b * 255);

		return stream.str();
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
