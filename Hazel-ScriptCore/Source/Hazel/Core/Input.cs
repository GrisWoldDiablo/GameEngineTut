namespace Hazel
{
	public class Input
	{
		#region Keyboard
		public static bool IsKeyPressed(KeyCode keyCode)
		{
			return InternalCalls.Input_IsKeyPressed(keyCode);
		}

		public static bool IsKeyDown(KeyCode keyCode)
		{
			return InternalCalls.Input_IsKeyDown(keyCode);
		}

		public static bool IsKeyUp(KeyCode keyCode)
		{
			return InternalCalls.Input_IsKeyUp(keyCode);
		}
		#endregion

		#region Mouse
		public static bool IsMouseButtonPressed(MouseCode mouseCode)
		{
			return InternalCalls.Input_IsMouseButtonPressed(mouseCode);
		}

		public static bool IsMouseButtonDown(MouseCode mouseCode)
		{
			return InternalCalls.Input_IsMouseButtonDown(mouseCode);
		}

		public static bool IsMouseButtonUp(MouseCode mouseCode)
		{
			return InternalCalls.Input_IsMouseButtonUp(mouseCode);
		}
		#endregion
	}
}
