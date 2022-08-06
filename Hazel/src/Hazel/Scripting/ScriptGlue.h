#pragma once
#include "Hazel/Scene/Components.h"

namespace Hazel
{
	class ScriptGlue
	{
	public:
		static void RegisterComponents();
		static void RegisterFunctions();

	private:
		template<typename... TComponent>
		static void RegisterComponent();
		template<typename... TComponent>
		static void RegisterComponents(ComponentGroup<TComponent...>);
	};
}
