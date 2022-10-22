#include "EditorResourceManager.h"

namespace Hazel::Utils
{
	struct EditorResourceData
	{
		std::map<EditorImage_, Ref<Texture2D>> Textures;
	};

	static EditorResourceData* sEditorResourceData = nullptr;

	void EditorResourceManager::Init()
	{
		sEditorResourceData = new EditorResourceData();

		sEditorResourceData->Textures =
		{
			{ Icon_Pan,				Texture2D::Create("Resources/Icons/Gizmo/PanIcon256White.png")		 },
			{ Icon_Magnifier,		Texture2D::Create("Resources/Icons/Gizmo/MagnifierIcon256White.png") },
			{ Icon_Eye,				Texture2D::Create("Resources/Icons/Gizmo/EyeIcon256White.png")		 },
			{ Icon_Nothing,			Texture2D::Create("Resources/Icons/Gizmo/NothingGizmo256White.png")	 },
			{ Icon_Position,		Texture2D::Create("Resources/Icons/Gizmo/PositionGizmo256White.png") },
			{ Icon_Rotation,		Texture2D::Create("Resources/Icons/Gizmo/RotationGizmo256White.png") },
			{ Icon_Scale,			Texture2D::Create("Resources/Icons/Gizmo/ScaleGizmo256White.png")	 },
			{ Icon_Local,			Texture2D::Create("Resources/Icons/Gizmo/LocalGizmo256White.png")	 },
			{ Icon_Global,			Texture2D::Create("Resources/Icons/Gizmo/GlobalGizmo256White.png")	 },
			{ Icon_Play,			Texture2D::Create("Resources/Icons/General/PlayButton256.png")		 },
			{ Icon_Stop,			Texture2D::Create("Resources/Icons/General/StopButton256.png")		 },
			{ Icon_Simulate,		Texture2D::Create("Resources/Icons/General/SimulateButton256.png")	 },
			{ Icon_Lock,			Texture2D::Create("Resources/Icons/General/Lock256.png")			 },
			{ Icon_Unlock,			Texture2D::Create("Resources/Icons/General/Unlock256.png")			 },
			{ Image_ShaderLoading,	Texture2D::Create("Resources/ShadersLoading.png")					 },
			{ Icon_Folder,			Texture2D::Create("Resources/Icons/ContentBrowser/FolderIcon256.png")},
			{ Icon_File,			Texture2D::Create("Resources/Icons/ContentBrowser/FileIcon256.png")	 },
		};
	}

	void EditorResourceManager::Shutdown()
	{
		delete sEditorResourceData;
	}

	Ref<Texture2D> EditorResourceManager::GetTexture(EditorImage_ editorImage)
	{
		if (sEditorResourceData->Textures.find(editorImage) == sEditorResourceData->Textures.end())
		{
			HZ_ASSERT_ONCE(false, "Editor Image not found!");
			return Texture2D::ErrorTexture;
		}

		return sEditorResourceData->Textures.at(editorImage);
	}
}
