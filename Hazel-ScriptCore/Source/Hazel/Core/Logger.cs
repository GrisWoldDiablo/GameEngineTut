namespace Hazel
{
	public class Logger
	{
		public static void Trace(string message)
		{
			InternalCalls.Logger_Trace(message);
		}

		public static void Debug(string message)
		{
			InternalCalls.Logger_Debug(message);
		}

		public static void Info(string message)
		{
			InternalCalls.Logger_Info(message);
		}

		public static void Warning(string message)
		{
			InternalCalls.Logger_Warning(message);
		}

		public static void Error(string message)
		{
			InternalCalls.Logger_Error(message);
		}

		public static void Critical(string message)
		{
			InternalCalls.Logger_Critical(message);
		}
	}
}
