#pragma once

namespace Hazel
{
	class ScriptGlue
	{
	public:
		template<typename T>
		static void RegisterComponent();
		static void RegisterComponents();
		static void RegisterFunctions();
	};
}
