namespace Hazel
{
	public class Debug
	{
		public static void Log(string message)
		{
			InternalCalls.Logger_Log(message);
		}
	}
}
