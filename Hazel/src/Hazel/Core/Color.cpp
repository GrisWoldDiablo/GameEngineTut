#include "hzpch.h"
#include "Color.h"
#include "Random.h"
#include "Hazel/Math/Math.h"

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
	const Color Color::Orange = Color(1.0f, 0.5f, 0.0f, 1.0f);

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
		double HPrime = H * 6.0;
		double X = C * (1 - abs(fmod(HPrime, 2) - 1));
		double m = V - C;
		double R, G, B;

		switch (static_cast<int>(glm::floor(HPrime)))
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

		return { (R + m), (G + m), (B + m), static_cast<double>(hsv.a) };
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

		if (Math::IsNearlyZero(C))
		{
			H = 0.0;
		}
		else if (Math::IsNearlyEqual(Xmax, r))
		{
			H = fmod(60.0 * ((g - b) / C) + 360.0, 360.0);
		}
		else if (Math::IsNearlyEqual(Xmax, g))
		{
			H = fmod(60.0 * ((b - r) / C) + 120.0, 360.0);
		}
		else if (Math::IsNearlyEqual(Xmax, b))
		{
			H = fmod(60.0 * ((r - g) / C) + 240.0, 360.0);
		}

		H /= 360.0;

		if (Math::IsNearlyZero(Xmax))
		{
			S = 0.0;
		}
		else
		{
			S = C / Xmax;
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

		double H = 0.0;
		double S = 0.0;
		double L = 0.0;
		double r = color.r;
		double g = color.g;
		double b = color.b;

		double Xmax = glm::max(glm::max(r, g), b);
		double Xmin = glm::min(glm::min(r, g), b);
		double C = Xmax - Xmin;

		if (Math::IsNearlyZero(C))
		{
			H = 0.0;
		}
		else if (Math::IsNearlyEqual(Xmax, r))
		{
			H = fmod(60 * ((g - b) / C) + 360.0, 360.0);
		}
		else if (Math::IsNearlyEqual(Xmax, g))
		{
			H = fmod(60 * ((b - r) / C) + 120.0, 360.0);
		}
		else if (Math::IsNearlyEqual(Xmax, b))
		{
			H = fmod(60 * ((r - g) / C) + 240.0, 360.0);
		}

		H /= 360.0;

		if (Math::IsNearlyZero(Xmax))
		{
			S = 0.0;
		}
		else
		{
			S = C / Xmax;
		}

		L = (Xmax - Xmin) / 2.0;

		return { H, S, L , color.a };
	}

	Color Color::RGBtoGrayscale(const Color& color)
	{
		HZ_PROFILE_FUNCTION();

		auto grayscaleValue = color.GetGrayscaleValue();
		return  { grayscaleValue, grayscaleValue, grayscaleValue, color.a };
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
				HZ_CORE_ASSERT(0, std::string("[] is not a valid Hexadecimal character.").insert(1, 1, character));
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

		return { red, green, blue };
	}

	Color Color::Random()
	{
		HZ_PROFILE_FUNCTION();

		return { Random::Float(), Random::Float(), Random::Float(), Random::Float() };
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
		return { red, green, blue, alpha };
	}

	Color::Color(float value)
		:Color(value, value, value)
	{}

	Color::Color(glm::vec4 value)
		: Color(value.r, value.g, value.b, value.a)
	{}

	Color::Color(int red, int green, int blue, float alpha)
		: Color(static_cast<float>(red) / 255.0f, static_cast<float>(green) / 255.0f, static_cast<float>(blue) / 255.0f, alpha)
	{}

	Color::Color(float red, float green, float blue, float alpha)
		: r(red), g(green), b(blue), a(alpha)
	{}

	Color::Color(double red, double green, double blue, double alpha)
		: Color(static_cast<float>(red), static_cast<float>(green), static_cast<float>(blue), static_cast<float>(alpha))
	{}

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
			<< std::setfill('0') << std::setw(2) << static_cast<int>(r * 255)
			<< std::setfill('0') << std::setw(2) << static_cast<int>(g * 255)
			<< std::setfill('0') << std::setw(2) << static_cast<int>(b * 255);

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
		case 1:
			return g;
		case 2:
			return b;
		case 3:
			return a;
		}

		HZ_CORE_ASSERT(0, "Color index [" + std::to_string(index) + "] out of bounds!");
		throw std::out_of_range("Index out of bounds!");
	}

	float Color::operator[](int index) const
	{
		switch (index)
		{
		case 0:
			return r;
		case 1:
			return g;
		case 2:
			return b;
		case 3:
			return a;
		}

		HZ_CORE_ASSERT(0, "Color index [" + std::to_string(index) + "] out of bounds!");
		throw std::out_of_range("Index out of bounds!");
	}

	Color Color::operator+(const Color& other) const
	{
		auto red = glm::clamp(this->r + other.r, 0.0f, 1.0f);
		auto green = glm::clamp(this->g + other.g, 0.0f, 1.0f);
		auto blue = glm::clamp(this->b + other.b, 0.0f, 1.0f);
		auto alpha = glm::clamp(this->a + other.a, 0.0f, 1.0f);
		return { red, green, blue, alpha };
	}

	Color Color::operator-(const Color& other)const
	{
		auto red = glm::clamp(this->r - other.r, 0.0f, 1.0f);
		auto green = glm::clamp(this->g - other.g, 0.0f, 1.0f);
		auto blue = glm::clamp(this->b - other.b, 0.0f, 1.0f);
		auto alpha = glm::clamp(this->a - other.a, 0.0f, 1.0f);
		return { red, green, blue, alpha };
	}

	Color Color::operator*(const Color& other)const
	{
		auto red = this->r * other.r;
		auto green = this->g * other.g;
		auto blue = this->b * other.b;
		auto alpha = this->a * other.a;
		return { red, green, blue, alpha };
	}

	Color Color::operator/(float value)const
	{
		HZ_CORE_ASSERT(!Math::IsNearlyZero(value), "Cannot divive by Zero!");
		auto red = this->r / value;
		auto green = this->g / value;
		auto blue = this->b / value;
		auto alpha = this->a / value;
		return { red, green, blue, alpha };
	}

	Color& Color::operator+=(const Color& other)
	{
		this->r = glm::clamp(this->r + other.r, 0.0f, 1.0f);
		this->g = glm::clamp(this->g + other.g, 0.0f, 1.0f);
		this->b = glm::clamp(this->b + other.b, 0.0f, 1.0f);
		this->a = glm::clamp(this->a + other.a, 0.0f, 1.0f);
		return *this;
	}

	Color& Color::operator-=(const Color& other)
	{
		this->r = glm::clamp(this->r - other.r, 0.0f, 1.0f);
		this->g = glm::clamp(this->g - other.g, 0.0f, 1.0f);
		this->b = glm::clamp(this->b - other.b, 0.0f, 1.0f);
		this->a = glm::clamp(this->a - other.a, 0.0f, 1.0f);
		return *this;
	}

	Color& Color::operator*=(const Color& other)
	{
		this->r *= other.r;
		this->g *= other.g;
		this->b *= other.b;
		this->a *= other.a;
		return *this;
	}

	Color& Color::operator/=(float value)
	{
		HZ_CORE_ASSERT(!Math::IsNearlyZero(value), "Cannot divive by Zero!");
		this->r /= value;
		this->g /= value;
		this->b /= value;
		this->a /= value;
		return *this;
	}

	bool Color::operator==(const Color& other) const
	{
		return Math::IsNearlyEqual(this->r, other.r)
			&& Math::IsNearlyEqual(this->g, other.g)
			&& Math::IsNearlyEqual(this->b, other.b)
			&& Math::IsNearlyEqual(this->a, other.a);
	}
}
