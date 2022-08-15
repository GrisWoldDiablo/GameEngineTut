using System;

namespace Hazel
{
	public struct Color : IEquatable<Color>
	{
		public float R; // Red
		public float G; // Green
		public float B; // Blue
		public float A; // Alpha

		Color(float red, float green, float blue, float alpha = 1.0f)
		{
			R = red;
			G = green;
			B = blue;
			A = alpha;
		}

		public Color(float value) : this(value, value, value, value) { }

		public Color(Vector4 value) : this(value.X, value.Y, value.Z, value.W) { }

		/// <summary>
		/// Return the Grayscale value of the color. (ITU-R BT.601 conversion)
		/// (0.299 * r) + (0.587 * g) + (0.114 * b)
		/// </summary>
		float GetGrayscaleValue()
		{
			return (0.299f * R) + (0.587f * G) + (0.114f * B);
		}

		/// <summary>
		/// Return hexadecimal version of the RGB
		/// </summary>
		string GetHexValue()
		{
			return $"{(int)(R * 255.0f):x}{(int)(G * 255.0f):x}{(int)(B * 255.0f):x}".ToUpper();
		}

		public override string ToString()
		{
			return $"({R:f3},{G:f3},{B:f3},{A:f3})";
		}

		public override int GetHashCode()
		{
			return (R.GetHashCode() << 2) ^ (G.GetHashCode() >> 1) ^ (B.GetHashCode() << 2) ^ A.GetHashCode();
		}

		public override bool Equals(object other)
		{
			if (!(other is Color))
			{
				return false;
			}

			return Equals((Color)other);
		}

		public bool Equals(Color other)
		{
			return R.Equals(other.R) && G.Equals(other.G) && B.Equals(other.B) && A.Equals(other.A);
		}

		///// <summary>
		///// Returns r, g, b, a using [0], [1], [2], [3] respectively.
		///// </summary>
		public float this[int index]
		{
			get
			{
				switch (index)
				{
					case 0: return R;
					case 1: return G;
					case 2: return B;
					case 3: return A;
					default:
						throw new System.IndexOutOfRangeException($"{nameof(Color)} invalid index!");
				}

			}
			set
			{
				switch (index)
				{
					case 0:
						R = value;
						break;
					case 1:
						G = value;
						break;
					case 2:
						B = value;
						break;
					case 3:
						A = value;
						break;
					default:
						throw new System.IndexOutOfRangeException($"{nameof(Color)} invalid index!");
				}
			}
		}

		#region statics
		public static Color Black => new Color(0.0f, 0.0f, 0.0f, 1.0f);           // [0x0000FF]
		public static Color White => new Color(1.0f, 1.0f, 1.0f, 1.0f);           // [0xFFFFFF]
		public static Color Clear => new Color(0.0f, 0.0f, 0.0f, 0.0f);           // [0x000000]
		public static Color Red => new Color(1.0f, 0.0f, 0.0f, 1.0f);             // [0xFF0000]
		public static Color Green => new Color(0.0f, 1.0f, 0.0f, 1.0f);           // [0x00FF00]
		public static Color Blue => new Color(0.0f, 0.0f, 1.0f, 1.0f);            // [0x0000FF]
		public static Color Cyan => new Color(0.0f, 1.0f, 1.0f, 1.0f);            // [0x00FFFF]
		public static Color Gray => new Color(0.5f, 0.5f, 0.5f, 1.0f);            // [0x7F7F7F]
		public static Color Grey => new Color(0.5f, 0.5f, 0.5f, 1.0f);            // [0x7F7F7F]
		public static Color Magenta => new Color(1.0f, 0.0f, 1.0f, 1.0f);         // [0xFF00FF]
		public static Color Turquoise => new Color(0.251f, 0.878f, 0.816f, 1.0f); // [0x40E0D0]
		public static Color Yellow => new Color(1.0f, 0.92f, 0.016f, 1.0f);       // [0xFFea04]
		public static Color Orange => new Color(1.0f, 0.5f, 0.0f, 1.0f);          // [0xFF7F00]

		/// <summary>
		/// Multiply each components by a value.
		/// </summary>
		public static Color operator *(Color value, float scalar)
		{
			return new Color(value.R * scalar, value.G * scalar, value.B * scalar, value.A * scalar);
		}

		/// <summary>
		/// Multiply each components by a value.
		/// </summary>
		public static Color operator *(float scalar, Color value)
		{
			return new Color(value.R * scalar, value.G * scalar, value.B * scalar, value.A * scalar);
		}

		/// <summary>
		/// Divide each components by a value.
		/// </summary>
		public static Color operator /(Color value, float divisor)
		{
			if (divisor.IsNearlyZero())
			{
				return Clear;
			}
			return new Color(value.R / divisor, value.G / divisor, value.B / divisor, value.A / divisor);
		}

		/// <summary>
		/// Add to each components a value.
		/// </summary>
		public static Color operator +(Color value, float addend)
		{
			return new Color(value.R + addend, value.G + addend, value.B + addend, value.A + addend);
		}

		/// <summary>
		/// Substract to each components a value.
		/// </summary>
		public static Color operator -(Color value, float minuend)
		{
			return new Color(value.R - minuend, value.G - minuend, value.B - minuend, value.A - minuend);
		}

		/// <summary>
		/// Add each components separately.
		/// </summary>
		public static Color operator +(Color lhs, Color rhs)
		{
			return new Color(lhs.R + rhs.R, lhs.G + rhs.G, lhs.B + rhs.B, lhs.A + rhs.A);
		}

		/// <summary>
		/// Subtract each components separately.
		/// </summary>
		public static Color operator -(Color lhs, Color rhs)
		{
			return new Color(lhs.R - rhs.R, lhs.G - rhs.G, lhs.B - rhs.B, lhs.A - rhs.A);
		}

		/// <summary>
		/// Multiply each components separately.
		/// </summary>
		public static Color operator *(Color lhs, Color rhs)
		{
			return new Color(lhs.R * rhs.R, lhs.G * rhs.G, lhs.B * rhs.B, lhs.A * rhs.A);
		}

		/// <summary>
		/// Compare each components separately.
		/// </summary>
		public static bool operator !=(Color lhs, Color rhs)
		{
			return (lhs.R != rhs.R &&
					lhs.G != rhs.G &&
					lhs.B != rhs.B &&
					lhs.A != rhs.A);
		}

		/// <summary>
		/// Compare each components separately.
		/// </summary>
		public static bool operator ==(Color lhs, Color rhs)
		{
			return (lhs.R == rhs.R &&
					lhs.G == rhs.G &&
					lhs.B == rhs.B &&
					lhs.A == rhs.A);
		}

		/// <summary>
		/// Covert from Hue, Saturation, Value/Brightness to Red Green Blue Color with alpha
		/// </summary>
		/// <param name="h">Hue</param>
		/// <param name="s">Saturation</param>
		/// <param name="v">Value/Brightness</param>
		/// <param name="a">Alpha</param>
		public static Color HSVtoRGB(float h, float s, float v, float a = 1.0f)
		{
			return HSVtoRGB(new Vector4(h, s, v, a));
		}

		/// <summary>
		/// Covert from vec4 to RGB Color
		/// <para> vec4.x = Hue				</para>
		/// <para> vec4.y = Saturation		</para>
		/// <para> vec4.z = Value/Brightness</para>
		/// <para> vec4.w = Alpha			</para>
		/// </summary>
		/// <param name="hsv">x(Hue), y(Saturation), z(Value/Brightness)</param>
		public static Color HSVtoRGB(Vector4 hsv)
		{
			double H = hsv.X;
			double S = hsv.Y;
			double V = hsv.Z;

			double C = S * V;
			double HPrime = H * 6.0;
			double X = C * (1 - Math.Abs(Math.IEEERemainder(HPrime, 2) - 1));
			double m = V - C;
			double R, G, B;

			switch (Math.Floor(HPrime))
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

			return new Color((float)(R + m), (float)(G + m), (float)(B + m), hsv.W);
		}

		/// <summary>
		/// From Red Green Blue in extracts the Hue, Saturation, Value/Brightness
		/// </summary>
		/// <param name="color">RGBA Color</param>
		/// <param name="H">Hue</param>
		/// <param name="S">Saturation</param>
		/// <param name="V">Value/Brightness</param>
		public static void RGBtoHSV(Color color, out float H, out float S, out float V)
		{
			var hsv = RGBtoHSV(color);
			H = hsv.X;
			S = hsv.Y;
			V = hsv.Z;
		}

		/// <summary>
		/// Convert from Red Green Blue Color to Hue, Saturation, Value/Brightness
		/// <para> vec4.x = Hue				</para>
		/// <para> vec4.y = Saturation		</para>
		/// <para> vec4.z = Value/Brightness</para>
		/// <para> vec4.w = Alpha			</para>
		/// </summary>
		/// <param name="color">RGBA Color</param>
		public static Vector4 RGBtoHSV(Color color)
		{
			double H = 0;
			double S = 0;
			double V = 0;
			double r = color.R;
			double g = color.G;
			double b = color.B;

			double Xmax = Math.Max(Math.Max(r, g), b);
			double Xmin = Math.Min(Math.Min(r, g), b);
			double C = Xmax - Xmin;

			if (C.IsNearlyZero())
			{
				H = 0.0;
			}
			else if (Xmax.IsNearlyEqual(r))
			{
				H = Math.IEEERemainder(60.0 * ((g - b) / C) + 360.0, 360.0);
			}
			else if (Xmax.IsNearlyEqual(g))
			{
				H = Math.IEEERemainder(60.0 * ((b - r) / C) + 120.0, 360.0);
			}
			else if (Xmax.IsNearlyEqual(b))
			{
				H = Math.IEEERemainder(60.0 * ((r - g) / C) + 240.0, 360.0);
			}

			H /= 360.0;

			if (Xmax.IsNearlyZero())
			{
				S = 0.0;
			}
			else
			{
				S = C / Xmax;
			}
			V = Xmax;

			return new Vector4((float)H, (float)S, (float)V, color.A);
		}

		/// <summary>
		/// From Red Green Blue in extracts the Hue, Saturation, Value/Brightness
		/// </summary>
		/// <param name="color">RGBA Color</param>
		/// <param name="H">Hue</param>
		/// <param name="S">Saturation</param>
		/// <param name="L">Lightness</param>
		public static void RGBtoHSL(Color color, out float H, out float S, out float L)
		{
			var hsl = RGBtoHSL(color);
			H = hsl.X;
			S = hsl.Y;
			L = hsl.Z;
		}

		/// <summary>
		/// Convert from Red Green Blue Color to Hue, Saturation, Value/Brightness
		/// <para> vec4.x = Hue				</para>
		/// <para> vec4.y = Saturation		</para>
		/// <para> vec4.z = Lightness		</para>
		/// <para> vec4.w = Alpha			</para>
		/// </summary>
		/// <param name="color">RGB Color</param>
		/// 
		public static Vector4 RGBtoHSL(Color color)
		{
			double H = 0.0;
			double S = 0.0;
			double L = 0.0;
			double r = color.R;
			double g = color.G;
			double b = color.B;

			double Xmax = Math.Max(Math.Max(r, g), b);
			double Xmin = Math.Min(Math.Min(r, g), b);
			double C = Xmax - Xmin;

			if (C.IsNearlyZero())
			{
				H = 0.0;
			}
			else if (Xmax.IsNearlyEqual(r))
			{
				H = Math.IEEERemainder(60 * ((g - b) / C) + 360.0, 360.0);
			}
			else if (Xmax.IsNearlyEqual(g))
			{
				H = Math.IEEERemainder(60 * ((b - r) / C) + 120.0, 360.0);
			}
			else if (Xmax.IsNearlyEqual(b))
			{
				H = Math.IEEERemainder(60 * ((r - g) / C) + 240.0, 360.0);
			}

			H /= 360.0;

			if (Xmax.IsNearlyZero())
			{
				S = 0.0;
			}
			else
			{
				S = C / Xmax;
			}

			L = (Xmax - Xmin) / 2.0;

			return new Vector4((float)H, (float)S, (float)L, color.A);
		}

		/// <summary>
		/// Convert from Red Green Blue Color to GrayScale version.
		/// </summary>
		/// <param name="color">RGB Color</param>
		public static Color RGBtoGrayscale(Color color)
		{
			var grayscaleValue = color.GetGrayscaleValue();
			return new Color(grayscaleValue, grayscaleValue, grayscaleValue, color.A);
		}

		/// <summary>
		/// Covert from hex to color
		/// <para> "FFFFFF" = White </para>
		/// </summary>
		/// <param name="hexValue">Hexadecimal value</param>
		public static Color HEXtoRGB(string hexValue)
		{
			if (hexValue.Length != 6)
			{
				throw new FormatException("RGB Hexadecimal value need to consist of 6 characters.");
			}

			foreach (var character in hexValue)
			{
				switch ((int)char.ToUpper(character))
				{
					case 0:
					case 1:
					case 2:
					case 3:
					case 4:
					case 5:
					case 6:
					case 7:
					case 8:
					case 9:
					case 'A':
					case 'B':
					case 'C':
					case 'D':
					case 'E':
					case 'F':
						break;
					default:
						throw new FormatException($"[{character}] is not a valid Hexadecimal character.");
				}
			}

			float red = float.Parse(hexValue.Substring(0, 2), System.Globalization.NumberStyles.HexNumber);
			float green = float.Parse(hexValue.Substring(2, 2), System.Globalization.NumberStyles.HexNumber);
			float blue = float.Parse(hexValue.Substring(4, 2), System.Globalization.NumberStyles.HexNumber);

			return new Color(red, green, blue);
		}

		/// <summary>
		/// Generate a random value for each components.
		/// </summary>
		public static Color Random()
		{
			// TODO use own Random Class.
			var rand = new Random();
			return new Color(
				(float)rand.NextDouble(),   // red
				(float)rand.NextDouble(),   // green
				(float)rand.NextDouble(),   // blue
				(float)rand.NextDouble());  // alpha
		}

		/// <summary>
		/// Returns a color lerped from colorA to colorB clamped
		/// Values returned cannot be lower than 0.0f and higher than 1.0f
		/// </summary>
		/// <param name="colorA">from color</param>
		/// <param name="colorB">to color</param>
		/// <param name="lerpAmount">from to percentage</param>
		public static Color Lerp(Color colorA, Color colorB, float lerpAmount)
		{
			var color = LerpUnclamped(colorA, colorB, lerpAmount);
			color = Clamp(color);
			return color;
		}

		/// <summary>
		/// Returns a color lerped from colorA to colorB
		/// Values returned can be lower than 0.0f and higher than 1.0f
		/// </summary>
		/// <param name="colorA">from color</param>
		/// <param name="colorB">to color</param>
		/// <param name="lerpValue">from to percentage</param>
		public static Color LerpUnclamped(Color colorA, Color colorB, float lerpValue)
		{

			var red = (colorB.R * lerpValue) + (colorA.R * (1.0f - lerpValue));
			var green = (colorB.G * lerpValue) + (colorA.G * (1.0f - lerpValue));
			var blue = (colorB.B * lerpValue) + (colorA.B * (1.0f - lerpValue));
			var alpha = (colorB.A * lerpValue) + (colorA.A * (1.0f - lerpValue));
			return new Color(red, green, blue, alpha);
		}

		/// <summary>
		/// Clamp each components between 0.0f and 1.0f;
		/// </summary>
		/// <param name="color">color to clamp</param>
		/// <returns>clamped color</returns>
		private static Color Clamp(Color color)
		{
			var r = HMath.Clamp(color.R, 0.0f, 1.0f);
			var g = HMath.Clamp(color.G, 0.0f, 1.0f);
			var b = HMath.Clamp(color.B, 0.0f, 1.0f);
			var a = HMath.Clamp(color.A, 0.0f, 1.0f);
			return new Color(r, g, b, a);
		}

		public static implicit operator Color(Vector4 value)
		{
			return new Color(value);
		}
		#endregion
	}
}
