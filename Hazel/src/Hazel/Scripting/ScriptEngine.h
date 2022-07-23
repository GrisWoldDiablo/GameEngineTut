#pragma once

namespace Hazel
{
	class ScriptEngine
	{
	public:
		static void Init();
		static void Shutdown();
		static bool TryReload();

	private:
		static void InitMono();
		static void ShutdownMono();
		static bool TryCreateAppDomain();

		static void DemoFunctionality();
	};
}
