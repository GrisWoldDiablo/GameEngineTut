#pragma once

#include "Hazel.h"

namespace Hazel::Utils
{
	enum EditorImage_
	{
		Icon_Pan,
		Icon_Magnifier,
		Icon_Eye,
		Icon_Nothing,
		Icon_Position,
		Icon_Rotation,
		Icon_Scale,
		Icon_Local,
		Icon_Global,
		Icon_Play,
		Icon_Stop,
		Icon_Simulate,
		Icon_Lock,
		Icon_Unlock,
		Image_ShaderLoading,
		Icon_Folder,
		Icon_File
	};

	class EditorResourceManager
	{
	public:
		static void Init();
		static void Shutdown();

		static Ref<Texture2D> GetTexture(EditorImage_ editorImage);
	};

	using ERM = EditorResourceManager;
}
