#pragma once

#include "Hazel/Scene/Entity.h"

extern "C" // Forward declare of class from C
{
	typedef struct _MonoClass MonoClass;
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoMethod MonoMethod;
	typedef struct _MonoAssembly MonoAssembly;
	typedef struct _MonoImage MonoImage;
	typedef struct _MonoAssemblyName MonoAssemblyName;
	typedef struct _MonoClassField MonoClassField;
}

namespace Hazel
{
	// https://docs.microsoft.com/en-us/dotnet/csharp/language-reference/language-specification/types#836-integral-types
	// TODO move to different header.
	enum class ScriptFieldType
	{
		None = 0,
		Float, Double, Char, Bool,
		SByte, Short, Int, Long,
		Byte, UShort, UInt, ULong,
		
		Vector2, Vector3, Vector4,
		Color,
		Entity
	};

	class Scene;
	class ScriptClass;
	class ScriptInstance;

	class ScriptEngine
	{
	public:
		static void Init();
		static void Shutdown();

		static void OnRuntimeStart(Scene* scene);
		static void OnRuntimeStop();
		static bool TryReload(bool shouldLog = true);

		static bool EntityClassExist(const std::string& fullClassName);
		static void OnCreateEntity(Entity entity);
		static void OnUpdateEntity(Entity entity, Timestep timestep);

		static Scene* GetSceneContext();
		static std::unordered_map<std::string, Ref<ScriptClass>> GetEntityClasses();
		static Ref<ScriptInstance> GetEntityScriptInstance(UUID entityID);

	private:
		static void InitMono();
		static void ShutdownMono();

		static void LoadAppDomain();
		static void UnloadAppDomain();
		static void ClearAssemblies();

		static bool TrySetupEngine();
		static bool TryLoadCoreAssembly(const std::filesystem::path& filePath);
		static bool TryLoadAppAssembly(const std::filesystem::path& filePath);
		static void LoadAssemblyClasses();

		static MonoObject* InstanciateClass(MonoClass* monoClass, MonoMethod* constructor = nullptr, void** params = nullptr);

		static MonoImage* GetCoreAssemblyImage();
		static MonoImage* GetAppAssemblyImage();
		static Ref<ScriptClass> GetEntityClass();

		friend class ScriptGlue;
		friend class ScriptClass;
		friend class ScriptInstance;
	};
}
