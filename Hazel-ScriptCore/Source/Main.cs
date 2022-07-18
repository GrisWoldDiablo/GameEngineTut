using System;

namespace Hazel
{
	public class Main
	{

		public float FloatVar { get; set; }

		public Main()
		{
			Console.WriteLine("Main Constructor!");
		}

		~Main()
		{
			Console.WriteLine("Main Destructor!");
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
	} 
}
