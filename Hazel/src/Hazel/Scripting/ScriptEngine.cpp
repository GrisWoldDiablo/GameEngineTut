#include "hzpch.h"
#include "Hazel/Scene/Scene.h"
#include "ScriptEngine.h"
#include "ScriptGlue.h"
#include "ScriptClass.h"
#include "ScriptInstance.h"

#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"
#include "mono/metadata/object.h"
#include "mono/metadata/tabledefs.h"

#include <string>

namespace Hazel
{
	namespace Utils
	{
		static std::unordered_map<std::string, ScriptFieldType> sScriptFieldTypeMap =
		{
			{ "System.Single"	,	ScriptFieldType::Float	 },
			{ "System.Double"	,	ScriptFieldType::Double	 },
			{ "System.Char"		,	ScriptFieldType::Char	 },
			{ "System.Boolean"	,	ScriptFieldType::Bool	 },

			{ "System.SByte"	,	ScriptFieldType::SByte	 },
			{ "System.Int16"	,	ScriptFieldType::Short	 },
			{ "System.Int32"	,	ScriptFieldType::Int	 },
			{ "System.Int64"	,	ScriptFieldType::Long	 },

			{ "System.Byte"		,	ScriptFieldType::Byte	 },
			{ "System.UInt16"	,	ScriptFieldType::UShort	 },
			{ "System.UInt32"	,	ScriptFieldType::UInt	 },
			{ "System.UInt64"	,	ScriptFieldType::ULong	 },

			{ "Hazel.Vector2"	,	ScriptFieldType::Vector2 },
			{ "Hazel.Vector3"	,	ScriptFieldType::Vector3 },
			{ "Hazel.Vector4"	,	ScriptFieldType::Vector4 },
			{ "Hazel.Color"		,	ScriptFieldType::Color	 },
			{ "Hazel.Entity"	,	ScriptFieldType::Entity	 },
		};

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
#if HZ_DEBUG
			auto monoAssemblyName = mono_assembly_get_name(assembly);
			auto assemblyName = mono_assembly_name_get_name(monoAssemblyName);
			HZ_CORE_LINFO("Mono Assembly: {0}", assemblyName);

			MonoImage* image = mono_assembly_get_image(assembly);
			const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
			int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

			for (int32_t i = 0; i < numTypes; i++)
			{
				uint32_t cols[MONO_TYPEDEF_SIZE];
				mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

				const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
				const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

				HZ_CORE_LTRACE("{0}.{1}", nameSpace, name);
			}
#endif // HZ_DEBUG
		}

		ScriptFieldType MonoTypeToScriptFieldType(MonoType* monoType)
		{
			std::string typeName = mono_type_get_name(monoType);
			auto it = sScriptFieldTypeMap.find(typeName);
			if (it == sScriptFieldTypeMap.end())
			{
				return ScriptFieldType::None;
			}

			return it->second;
		}

		std::string ScriptFieldTypeToString(ScriptFieldType scriptFieldType)
		{
			switch (scriptFieldType)
			{
			case ScriptFieldType::Float:  return "Float";
			case ScriptFieldType::Double: return "Double";
			case ScriptFieldType::Char:	  return "Char";
			case ScriptFieldType::Bool:	  return "Bool";

			case ScriptFieldType::SByte:  return "SByte";
			case ScriptFieldType::Short:  return "Short";
			case ScriptFieldType::Int:	  return "Int";
			case ScriptFieldType::Long:	  return "Long";

			case ScriptFieldType::Byte:	  return "Byte";
			case ScriptFieldType::UShort: return "UShort";
			case ScriptFieldType::UInt:	  return "UInt";
			case ScriptFieldType::ULong:  return "ULong";

			case ScriptFieldType::Vector2:return "Vector2";
			case ScriptFieldType::Vector3:return "Vector3";
			case ScriptFieldType::Vector4:return "Vector4";
			case ScriptFieldType::Color:  return "Color";
			case ScriptFieldType::Entity: return "Entity";
			}

			return "<Invalid>";
		}
	}

	struct StriptEngineData
	{
		MonoDomain* RootDomain = nullptr;
		MonoDomain* AppDomain = nullptr;

		MonoAssembly* CoreAssembly = nullptr;
		MonoImage* CoreAssemblyImage = nullptr;

		MonoAssembly* AppAssembly = nullptr;
		MonoImage* AppAssemblyImage = nullptr;

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

		Utils::PrintAssemblyType(sScriptData->CoreAssembly);
		Utils::PrintAssemblyType(sScriptData->AppAssembly);
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

		TryReload(false);
	}

	bool ScriptEngine::TryReload(bool shouldLog)
	{
		if (shouldLog)
		{
			HZ_CORE_LINFO("Reloading ScriptEngine");
		}

		ClearAssemblies();

		UnloadAppDomain();

		LoadAppDomain();

		if (!TrySetupEngine())
		{
			return false;
		}

		if (shouldLog)
		{
			HZ_CORE_LINFO("ScriptEngine Reloaded");

			Utils::PrintAssemblyType(sScriptData->CoreAssembly);
			Utils::PrintAssemblyType(sScriptData->AppAssembly);
		}

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

	Ref<ScriptInstance> ScriptEngine::GetEntityScriptInstance(UUID entityID)
	{
		auto it = sScriptData->EntityInstances.find(entityID);
		if (it == sScriptData->EntityInstances.end())
		{
			return nullptr;
		}

		return it->second;
	}

	void ScriptEngine::InitMono()
	{
		mono_set_assemblies_path("mono/lib");

		MonoDomain* rootDomain = mono_jit_init("HazelJITRuntime");
		HZ_CORE_ASSERT(rootDomain, "Root Domain could not be initialized!");

		// Store the root Domain
		sScriptData->RootDomain = rootDomain;

		LoadAppDomain();
	}

	void ScriptEngine::ShutdownMono()
	{
		UnloadAppDomain();

		mono_jit_cleanup(sScriptData->RootDomain);
		sScriptData->RootDomain = nullptr;
	}

	void ScriptEngine::LoadAppDomain()
	{
		constexpr char* domainName = "HazelScriptRuntime";
		MonoDomain* appDomain = mono_domain_create_appdomain(domainName, nullptr);
		HZ_ASSERT(mono_domain_set(appDomain, false), "Fail to load mono AppDomain");
		sScriptData->AppDomain = appDomain;
	}

	void ScriptEngine::UnloadAppDomain()
	{
		// Since we are unloading the current domain we need to set one before.
		mono_domain_set(sScriptData->RootDomain, false);

		mono_domain_unload(sScriptData->AppDomain);
		sScriptData->AppDomain = nullptr;
	}

	void ScriptEngine::ClearAssemblies()
	{
		sScriptData->AppAssembly = nullptr;
		sScriptData->AppAssemblyImage = nullptr;
		sScriptData->CoreAssembly = nullptr;
		sScriptData->CoreAssemblyImage = nullptr;

		sScriptData->EntityBaseClass.reset();
		sScriptData->EntityClasses.clear();
		sScriptData->EntityBaseClass.reset();
	}

	bool ScriptEngine::TrySetupEngine()
	{
		if (!TryLoadCoreAssembly("Resources/Scripts/Hazel-ScriptCore.dll"))
		{
			return false;
		}

		if (!TryLoadAppAssembly("SandboxProject/Assets/Scripts/Binaries/Sandbox.dll"))
		{
			return false;
		}

		LoadAssemblyClasses();

		ScriptGlue::RegisterComponents();
		ScriptGlue::RegisterFunctions();

		sScriptData->EntityBaseClass = CreateRef<ScriptClass>("Hazel", "Entity", true);

		return true;
	}

	bool ScriptEngine::TryLoadCoreAssembly(const std::filesystem::path& filePath)
	{
		MonoAssembly* coreAssembly = Utils::LoadMonoAssembly(filePath);

		if (!coreAssembly)
		{
			HZ_CORE_LCRITICAL("Failed to load assembly.");
			return false;
		}

		sScriptData->CoreAssembly = coreAssembly;
		sScriptData->CoreAssemblyImage = mono_assembly_get_image(coreAssembly);

		return true;
	}

	bool ScriptEngine::TryLoadAppAssembly(const std::filesystem::path& filePath)
	{
		MonoAssembly* appAssembly = Utils::LoadMonoAssembly(filePath);

		if (!appAssembly)
		{
			HZ_CORE_LCRITICAL("Failed to load assembly.");
			return false;
		}

		sScriptData->AppAssembly = appAssembly;
		sScriptData->AppAssemblyImage = mono_assembly_get_image(appAssembly);

		return true;
	}

	void ScriptEngine::LoadAssemblyClasses()
	{
		sScriptData->EntityClasses.clear();

		const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(sScriptData->AppAssemblyImage, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

		MonoClass* entityClass = mono_class_from_name(sScriptData->CoreAssemblyImage, "Hazel", "Entity");

		for (int32_t i = 0; i < numTypes; i++)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

			const char* nameSpace = mono_metadata_string_heap(sScriptData->AppAssemblyImage, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* className = mono_metadata_string_heap(sScriptData->AppAssemblyImage, cols[MONO_TYPEDEF_NAME]);

			std::string fullName;
			if (strlen(nameSpace) != 0)
			{
				fullName = fmt::format("{}.{}", nameSpace, className);
			}
			else
			{
				fullName = className;
			}

			MonoClass* monoClass = mono_class_from_name(sScriptData->AppAssemblyImage, nameSpace, className);
			if (!monoClass || monoClass == entityClass)
			{
				continue;
			}

			bool isEntity = mono_class_is_subclass_of(monoClass, entityClass, false);
			if (!isEntity)
			{
				continue;
			}

			Ref<ScriptClass> scriptClass = CreateRef<ScriptClass>(nameSpace, className);

			sScriptData->EntityClasses[fullName] = scriptClass;


			int fieldCounts = mono_class_num_fields(monoClass);
			HZ_CORE_LINFO("Class {0}", className);
			HZ_CORE_LDEBUG("  {1} fields: ", className, fieldCounts);
			void* iterator = nullptr;
			while (MonoClassField* field = mono_class_get_fields(monoClass, &iterator))
			{
				const auto type = mono_field_get_type(field);
				const auto scriptFieldType = Utils::MonoTypeToScriptFieldType(type);
				const auto typeName = Utils::ScriptFieldTypeToString(scriptFieldType);
				const auto fieldName = mono_field_get_name(field);
				const auto flags = mono_field_get_flags(field);
				const auto accessibility = flags & FIELD_ATTRIBUTE_FIELD_ACCESS_MASK;

				std::string extraAttribute = "";
				switch (flags & ~FIELD_ATTRIBUTE_FIELD_ACCESS_MASK)
				{
				case FIELD_ATTRIBUTE_STATIC:
					extraAttribute = "static";
					break;
				case FIELD_ATTRIBUTE_INIT_ONLY:
					extraAttribute = "readonly";
					break;
				default:
					break;
				}

				std::string access;
				switch (accessibility)
				{
				case FIELD_ATTRIBUTE_PUBLIC:
					access = "public";
					break;
				case FIELD_ATTRIBUTE_FAMILY:
					access = "protected";
					break;
				case FIELD_ATTRIBUTE_ASSEMBLY:
					access = "internal";
					break;
				case FIELD_ATTRIBUTE_PRIVATE:
					access = "private";
					break;
				default:
					access = "UNKNOWN";
					break;
				}

				HZ_CORE_LTRACE("    {0} {1} {2} ({3})", extraAttribute, access, fieldName, typeName);

				if (extraAttribute.empty() && access == "public")
				{
					scriptClass->_fields[fieldName] = { scriptFieldType, fieldName, field };
				}
			}

			// TODO Revisit property
			//int propertyCounts = mono_class_num_properties(monoClass);
			//HZ_CORE_LDEBUG("  {0} properties: ", propertyCounts);
			//iterator = nullptr;
			//while (MonoProperty* prop = mono_class_get_properties(monoClass, &iterator))
			//{
			//	HZ_CORE_LTRACE("    {0}", mono_property_get_name(prop));
			//}
		}
	}

	MonoObject* ScriptEngine::InstanciateClass(MonoClass* monoClass, MonoMethod* constructor, void** params)
	{
		MonoObject* monoObject = mono_object_new(sScriptData->AppDomain, monoClass);
		mono_runtime_object_init(monoObject);

		if (constructor)
		{
			mono_runtime_invoke(constructor, monoObject, params, nullptr);
		}

		return monoObject;
	}

	MonoImage* ScriptEngine::GetCoreAssemblyImage()
	{
		return sScriptData->CoreAssemblyImage;
	}

	MonoImage* ScriptEngine::GetAppAssemblyImage()
	{
		return sScriptData->AppAssemblyImage;
	}

	Ref<ScriptClass> ScriptEngine::GetEntityClass()
	{
		return sScriptData->EntityBaseClass;
	}
}
