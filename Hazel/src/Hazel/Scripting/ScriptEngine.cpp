#include "hzpch.h"
#include "Hazel/Scene/Scene.h"
#include "ScriptEngine.h"
#include "ScriptGlue.h"
#include "ScriptClass.h"
#include "ScriptInstance.h"

#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"
#include "mono/metadata/object.h"

#include <string>

namespace Hazel
{
	namespace Utils
	{
		// TODO: Move to FileSystem Class
		static char* ReadBytes(const std::filesystem::path& filePath, uint32_t* outSize)
		{
			std::ifstream stream(filePath, std::ios::binary | std::ios::ate);

			if (!stream)
			{
				HZ_CORE_LERROR("Fail to load filePath {0}!", filePath);
				return nullptr;
			}

			const std::streampos end = stream.tellg();
			stream.seekg(0, std::ios::beg);
			const auto size = static_cast<uint32_t>(end - stream.tellg());

			if (size == 0)
			{
				HZ_CORE_LERROR("File empty!");
				return nullptr;
			}

			char* buffer = new char[size];
			stream.read(buffer, size);
			stream.close();

			*outSize = size;

			return buffer;
		}

		static MonoAssembly* LoadMonoAssembly(const std::filesystem::path& filePath)
		{
			uint32_t fileSize = 0;
			char* fileData = ReadBytes(filePath.string(), &fileSize);
			if (!fileData)
			{
				return nullptr;
			}

			MonoImageOpenStatus status;
			MonoImage* image = mono_image_open_from_data_full(fileData, fileSize, 1, &status, 0);

			if (status != MONO_IMAGE_OK)
			{
				const char* errorMessage = mono_image_strerror(status);
				HZ_CORE_LERROR("Mono Error: {}", errorMessage);
				return nullptr;
			}

			MonoAssembly* assembly = mono_assembly_load_from_full(image, filePath.string().c_str(), &status, 0);
			mono_image_close(image);

			delete[] fileData;
			return assembly;
		}

		static void PrintAssemblyType(MonoAssembly* assembly)
		{
			MonoImage* image = mono_assembly_get_image(assembly);
			const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
			int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

			HZ_CORE_LINFO("Mono Assembly:");
			for (int32_t i = 0; i < numTypes; i++)
			{
				uint32_t cols[MONO_TYPEDEF_SIZE];
				mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

				const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
				const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

				HZ_CORE_LTRACE("{0}.{1}", nameSpace, name);
			}
		}
	}

	struct StriptEngineData
	{
		MonoDomain* RootDomain = nullptr;
		MonoDomain* AppDomain = nullptr;

		MonoAssembly* CoreAssembly = nullptr;
		MonoImage* CoreAssemblyImage = nullptr;

		Ref<ScriptClass> EntityBaseClass;

		std::unordered_map<std::string, Ref<ScriptClass>> EntityClasses;
		std::unordered_map<UUID, Ref<ScriptInstance>> EntityInstances;

		// Runtime
		Scene* SceneContext = nullptr;
	};

	static StriptEngineData* sScriptData = nullptr;

	void ScriptEngine::Init()
	{
		sScriptData = new StriptEngineData();

		InitMono();

		if (!TrySetupEngine())
		{
			return;
		}
	}

	void ScriptEngine::Shutdown()
	{
		ShutdownMono();

		delete sScriptData;
	}

	void ScriptEngine::OnRuntimeStart(Scene* scene)
	{
		sScriptData->SceneContext = scene;
	}

	void ScriptEngine::OnRuntimeStop()
	{
		sScriptData->SceneContext = nullptr;

		sScriptData->EntityInstances.clear();
	}

	bool ScriptEngine::TryReload()
	{
		HZ_CORE_LINFO("Reloading ScriptEngine");

		mono_domain_set(sScriptData->RootDomain, false);

		MonoObject* exc = nullptr;
		mono_domain_try_unload(sScriptData->AppDomain, &exc);
		if (exc)
		{
			return false;
		}

		if (!TrySetupEngine())
		{
			return false;
		}

		HZ_CORE_LINFO("ScriptEngine Reloaded");

		return true;
	}

	bool ScriptEngine::EntityClassExist(const std::string& fullClassName)
	{
		return sScriptData->EntityClasses.find(fullClassName) != sScriptData->EntityClasses.end();
	}

	void ScriptEngine::OnCreateEntity(Entity entity)
	{
		const auto& scriptComponent = entity.GetComponent<ScriptComponent>();
		if (EntityClassExist(scriptComponent.ClassName))
		{
			auto instance = CreateRef<ScriptInstance>(sScriptData->EntityClasses[scriptComponent.ClassName], entity);
			sScriptData->EntityInstances[entity.GetUUID()] = instance;

			instance->InvokeOnCreate();
		}
	}

	void ScriptEngine::OnUpdateEntity(Entity entity, Timestep timestep)
	{
		const auto& entityUUID = entity.GetUUID();
		HZ_CORE_ASSERT(sScriptData->EntityInstances.find(entityUUID) != sScriptData->EntityInstances.end(), "entity UUID [{0}] missing", entityUUID);

		auto instance = sScriptData->EntityInstances[entityUUID];
		instance->InvokeOnUpdate(static_cast<float>(timestep));
	}

	Scene* ScriptEngine::GetSceneContext()
	{
		return sScriptData->SceneContext;
	}

	std::unordered_map<std::string, Ref<ScriptClass>> ScriptEngine::GetEntityClasses()
	{
		return sScriptData->EntityClasses;
	}

	void ScriptEngine::InitMono()
	{
		mono_set_assemblies_path("mono/lib");

		MonoDomain* rootDomain = mono_jit_init("HazelJITRuntime");
		HZ_CORE_ASSERT(rootDomain, "MonoDomain could not be initialized!");

		// Store the root Domain
		sScriptData->RootDomain = rootDomain;
	}

	void ScriptEngine::ShutdownMono()
	{
		// Since we are unloading the current domain we need to set one before.
		mono_domain_set(sScriptData->RootDomain, false);

		mono_domain_unload(sScriptData->AppDomain);
		sScriptData->AppDomain = nullptr;

		mono_jit_cleanup(sScriptData->RootDomain);
		sScriptData->RootDomain = nullptr;
	}

	bool ScriptEngine::TrySetupEngine()
	{
		sScriptData->EntityBaseClass.reset();
		sScriptData->EntityClasses.clear();
		sScriptData->CoreAssembly = nullptr;

		if (!TryLoadCSharpAssembly("Resources/Scripts/Hazel-ScriptCore.dll"))
		{
			return false;
		}

		sScriptData->CoreAssemblyImage = mono_assembly_get_image(sScriptData->CoreAssembly);
		sScriptData->EntityBaseClass = CreateRef<ScriptClass>("Hazel", "Entity");
		LoadAssemblyClasses(sScriptData->CoreAssembly);

		ScriptGlue::RegisterComponents();
		ScriptGlue::RegisterFunctions();

		return true;
	}

	bool ScriptEngine::TryLoadCSharpAssembly(const std::filesystem::path& filePath)
	{
		constexpr char* domainName = "HazelScriptRuntime";
		MonoDomain* appDomain = mono_domain_create_appdomain(domainName, nullptr);
		if (!mono_domain_set(appDomain, false))
		{

			return false;
		}

		sScriptData->AppDomain = appDomain;
		MonoAssembly* coreAssembly = Utils::LoadMonoAssembly(filePath);

		if (!coreAssembly)
		{
			HZ_CORE_LCRITICAL("Failed to load assembly.");
			return false;
		}

		sScriptData->CoreAssembly = coreAssembly;

		Utils::PrintAssemblyType(sScriptData->CoreAssembly);

		return true;
	}

	void ScriptEngine::LoadAssemblyClasses(MonoAssembly* assembly)
	{
		sScriptData->EntityClasses.clear();

		MonoImage* image = mono_assembly_get_image(assembly);
		const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);
		MonoClass* entityClass = mono_class_from_name(image, "Hazel", "Entity");

		HZ_CORE_LINFO("Mono Assembly:");
		for (int32_t i = 0; i < numTypes; i++)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);


			const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

			std::string fullName;
			if (strlen(nameSpace) != 0)
			{
				fullName = fmt::format("{}.{}", nameSpace, name);
			}
			else
			{
				fullName = name;
			}

			MonoClass* monoClass = mono_class_from_name(image, nameSpace, name);
			if (monoClass == entityClass)
			{
				continue;
			}

			bool isEntity = mono_class_is_subclass_of(monoClass, entityClass, false);
			if (isEntity)
			{
				sScriptData->EntityClasses[fullName] = CreateRef<ScriptClass>(nameSpace, name);
			}
		}
	}

	MonoObject* ScriptEngine::InstanciateClass(MonoClass* monoClass, MonoMethod* constructor, void** params)
	{
		MonoObject* monoObject = mono_object_new(sScriptData->AppDomain, monoClass);
		if (constructor)
		{
			mono_runtime_invoke(constructor, monoObject, params, nullptr);
			return monoObject;
		}

		mono_runtime_object_init(monoObject);
		return monoObject;
	}

	MonoImage* ScriptEngine::GetCoreAssemblyImage()
	{
		return sScriptData->CoreAssemblyImage;
	}

	Ref<ScriptClass> ScriptEngine::GetEntityClass()
	{
		return sScriptData->EntityBaseClass;
	}
}
