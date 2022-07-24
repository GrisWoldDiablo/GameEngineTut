using System;

namespace Hazel
{
	public struct Vector3
	{
		public float X;
		public float Y;
		public float Z;

		public Vector3(float x, float y, float z)
		{
			X = x;
			Y = y;
			Z = z;
		}

		public override string ToString()
		{
			return $"({X:F2},{Y},{Z})";
		}
	}

	public class Entity
	{
		public Entity()
		{
			Console.WriteLine("Entity Constructor!");
			Log("GrisWold", 69420);

			var position = new Vector3(5f, 2.5f, 1.0f);
			Console.WriteLine($"Result = {Log(position)}");
			Console.WriteLine("{0}", InternalCalls.NativeLog_Vector3Dot(ref position));
			Console.WriteLine("{0}", InternalCalls.NativeLog_Vector3Struct(ref position));
		}

		~Entity()
		{
			Console.WriteLine("Entity Destructor!");
		}

		public void PrintMessage()
		{
			Console.WriteLine("Hello World from C#!");
		}

		public void PrintMessage(int value)
		{
			Console.WriteLine($"C# int: {value}");
		}

		public void PrintMessage(int value1, int value2)
		{
			Console.WriteLine($"C# ints: {value1} and {value2}");
		}

		public void PrintCustomMessage(string message)
		{
			Console.WriteLine($"C# string: {message}");
		}

		private void Log(string text, int parameter)
		{
			InternalCalls.NativeLog(text, parameter);
		}

		private Vector3 Log(Vector3 position)
		{
			InternalCalls.NativeLog_Vector3(ref position, out Vector3 result);
			return result;
		}
	}
}
