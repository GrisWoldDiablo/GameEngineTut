#include "hzpch.h"
#include "Hazel/Scene/Scene.h"
#include "Hazel/Core/Application.h"

#include "ScriptEngine.h"
#include "ScriptGlue.h"
#include "ScriptClass.h"
#include "ScriptInstance.h"

#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"
#include "mono/metadata/object.h"
#include "mono/metadata/tabledefs.h"
#include "mono/metadata/class.h"

#include "FileWatch.h"

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

			{ "System.String"	,	ScriptFieldType::String	 },

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
	}

	struct ScriptEngineData
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

		std::unordered_map<UUID, ScriptFieldMap> EntityScriptFields;

		// Runtime
		Scene* SceneContext = nullptr;

		// FileWatch
		Scope<filewatch::FileWatch<std::string>> CoreAssemblyFileWatcher;
		Scope<filewatch::FileWatch<std::string>> AppAssemblyFileWatcher;
		bool IsAssemblyReloading = false;
	};

	static ScriptEngineData* sScriptData = nullptr;

	static void OnAssemblyFileSystemEvent(const std::string& path, const filewatch::Event eventType)
	{
		if (!sScriptData->IsAssemblyReloading && eventType == filewatch::Event::modified)
		{
			sScriptData->IsAssemblyReloading = true;

			if (sScriptData->SceneContext)
			{
				HZ_CORE_LINFO("Queuing Script Assembly reload.");
				return;
			}

			using namespace std::chrono_literals;
			std::this_thread::sleep_for(500ms);

			Application::Get().SubmitToMainThread([]
			{
				ScriptEngine::TryReload();
			});
		}
	}

	void ScriptEngine::Init()
	{
		sScriptData = new ScriptEngineData();

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
		for (auto& [uuid, instance] : sScriptData->EntityInstances)
		{
			instance->InvokeOnDestroy();
		}

		sScriptData->SceneContext = nullptr;

		sScriptData->EntityInstances.clear();

		if (sScriptData->IsAssemblyReloading)
		{
			TryReload(false);
		}
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
		return sScriptData->EntityClasses.contains(fullClassName);
	}

	Ref<ScriptInstance> ScriptEngine::OnCreateEntity(Entity entity)
	{
		Ref<ScriptInstance> instance = nullptr;

		UUID entityUUID = entity.GetUUID();

		if (instance = GetEntityScriptInstance(entityUUID))
		{
			return instance;
		}

		if (!entity.HasComponent<ScriptComponent>())
		{
			instance = CreateRef<ScriptInstance>(sScriptData->EntityBaseClass, entity);
			sScriptData->EntityInstances[entityUUID] = instance;
			instance->InvokeOnCreate();
			return instance;
		}

		const auto& scriptComponent = entity.GetComponent<ScriptComponent>();
		if (EntityClassExist(scriptComponent.ClassName))
		{
			instance = CreateRef<ScriptInstance>(sScriptData->EntityClasses[scriptComponent.ClassName], entity);
			sScriptData->EntityInstances[entityUUID] = instance;

			// Copy field values
			if (sScriptData->EntityScriptFields.contains(entityUUID))
			{
				const auto& fieldMap = sScriptData->EntityScriptFields.at(entityUUID);

				for (const auto& [name, fieldInstance] : fieldMap)
				{
					switch (fieldInstance.Field.Type)
					{
					case  ScriptFieldType::String:
					{
						instance->TrySetFieldStringValueInternal(name, fieldInstance._stringData);
						break;
					}
					case  ScriptFieldType::Entity:
					{
						const auto fieldEntityUUID = fieldInstance.GetValue<uint64_t>();
						if (auto foundEntity = sScriptData->SceneContext->GetEntityByUUID(fieldEntityUUID))
						{
							const auto& foundEntityInstance = OnCreateEntity(foundEntity);
							HZ_ASSERT(foundEntityInstance, "Failed at Creating {0} Instance", foundEntity.Name());
							instance->TrySetFieldValueInternal(name, foundEntityInstance->_instance);
						}
						break;
					}
					default:
						instance->TrySetFieldValueInternal(name, fieldInstance._dataBuffer);

						break;
					}
				}
			}

			instance->InvokeOnCreate();
		}

		return instance;
	}

	void ScriptEngine::OnDestroyEntity(Entity entity)
	{
		const auto& entityUUID = entity.GetUUID();
		HZ_CORE_ASSERT(sScriptData->EntityInstances.contains(entityUUID), "Entity UUID [{0}] missing", entityUUID);

		auto& instance = sScriptData->EntityInstances.at(entityUUID);
		instance->InvokeOnDestroy();
		sScriptData->EntityInstances.erase(entityUUID);
	}

	void ScriptEngine::OnUpdateEntity(Entity entity, Timestep timestep)
	{
		const auto& entityUUID = entity.GetUUID();
		HZ_CORE_ASSERT(sScriptData->EntityInstances.contains(entityUUID), "Entity UUID [{0}] missing", entityUUID);

		auto& instance = sScriptData->EntityInstances.at(entityUUID);
		instance->InvokeOnUpdate(static_cast<float>(timestep));
	}

	Scene* ScriptEngine::GetSceneContext()
	{
		return sScriptData->SceneContext;
	}

	Ref<ScriptClass> ScriptEngine::GetEntityClass(const std::string& fullClassName)
	{
		if (EntityClassExist(fullClassName))
		{
			return sScriptData->EntityClasses.at(fullClassName);
		}

		return nullptr;
	}

	std::unordered_map<std::string, Ref<ScriptClass>> ScriptEngine::GetEntityClasses()
	{
		return sScriptData->EntityClasses;
	}

	ScriptFieldMap& ScriptEngine::GetScriptFieldMap(Entity entity)
	{
		HZ_CORE_ASSERT(entity, "Entity is invalid");

		return sScriptData->EntityScriptFields[entity.GetUUID()];
	}

	void ScriptEngine::EraseFromScriptFieldMap(Entity entity)
	{
		HZ_CORE_ASSERT(entity, "Entity is invalid");

		sScriptData->EntityScriptFields.erase(entity.GetUUID());
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
		constexpr char* domainName = const_cast<char*>("HazelScriptRuntime");
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

		sScriptData->IsAssemblyReloading = false;

		LoadAssemblyClasses();

		ScriptGlue::RegisterComponents();
		ScriptGlue::RegisterFunctions();

		// TODO ? Move somewhere else.
		{
			auto baseClass = CreateRef<ScriptClass>("Hazel", "Entity", true);

			const std::string fieldName = "Id";
			auto field = mono_class_get_field_from_name(baseClass->_monoClass, fieldName.c_str());
			baseClass->_fields[fieldName] = ScriptField{ ScriptFieldType::ULong, fieldName, field };

			sScriptData->EntityBaseClass = baseClass;
		}

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

		sScriptData->CoreAssemblyFileWatcher = CreateScope<filewatch::FileWatch<std::string>>(
			filePath.string(), &OnAssemblyFileSystemEvent);

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

		sScriptData->AppAssemblyFileWatcher = CreateScope<filewatch::FileWatch<std::string>>(
			filePath.string(), &OnAssemblyFileSystemEvent);

		return true;
	}

	void ScriptEngine::LoadAssemblyClasses()
	{
		MonoDomain* loadingDomain = mono_domain_create_appdomain(const_cast<char*>("loadingDomain"), nullptr);

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
				auto scriptFieldType = Utils::MonoTypeToScriptFieldType(type);

				if (scriptFieldType == ScriptFieldType::None)
				{
					if (auto* typeMonoClass = mono_type_get_class(type))
					{
						if (mono_class_is_subclass_of(typeMonoClass, entityClass, false))
						{
							scriptFieldType = ScriptFieldType::Entity;
						}
					}
				}

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

				std::string accessModifier;
				switch (accessibility)
				{
				case FIELD_ATTRIBUTE_PUBLIC:
					accessModifier = "public";
					break;
				case FIELD_ATTRIBUTE_FAMILY:
					accessModifier = "protected";
					break;
				case FIELD_ATTRIBUTE_ASSEMBLY:
					accessModifier = "internal";
					break;
				case FIELD_ATTRIBUTE_PRIVATE:
					accessModifier = "private";
					break;
				default:
					accessModifier = "UNKNOWN";
					break;
				}

				HZ_CORE_LTRACE("    {0} {1} {2} ({3})", extraAttribute, accessModifier, fieldName, typeName);

				if (extraAttribute.empty() && accessibility == FIELD_ATTRIBUTE_PUBLIC)
				{
					// TODO Revisit defaultFieldValue
					// should we keep a copy of the default value in ScriptField?
					// Since ScriptFieldInstance has a copy of ScriptField.

					static uint8_t defaultFieldDataBuffer[16];
					memset(defaultFieldDataBuffer, 0, sizeof(defaultFieldDataBuffer));

					MonoObject* monoObject = mono_object_new(loadingDomain, monoClass);
					mono_runtime_object_init(monoObject);

					ScriptField scriptField = { scriptFieldType, fieldName, field };

					switch (scriptFieldType)
					{
					case ScriptFieldType::String:
					{
						if (MonoObject* monoStringObject = mono_field_get_value_object(loadingDomain, field, monoObject))
						{
							MonoString* monoString = reinterpret_cast<MonoString*>(monoStringObject);
							scriptField.DefaultStringData = mono_string_to_utf8(monoString);
						}
						break;
					}
					case ScriptFieldType::Entity:
					{
						// Do nothing leave data buffer at 0
						break;
					}
					default: // All other types.
					{
						mono_field_get_value(monoObject, field, defaultFieldDataBuffer);
						memcpy_s(scriptField.DefaultData, sizeof(scriptField.DefaultData), defaultFieldDataBuffer, sizeof(defaultFieldDataBuffer));
						break;
					}
					}

					scriptClass->_fields[fieldName] = scriptField;
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

		mono_domain_unload(loadingDomain);
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

	bool ScriptEngine::IsBaseClass(const MonoClass* monoClass)
	{
		return monoClass == GetEntityClass()->_monoClass;
	}

	bool ScriptEngine::IsSubClassOf(MonoClass* child, MonoClass* parent, bool shouldCheckInterface)
	{
		return mono_class_is_subclass_of(child, parent, shouldCheckInterface);
	}

	bool ScriptEngine::IsSubClassOf(MonoClass* child, const std::string& parentFullClassName, bool shouldCheckInterface)
	{
		if (EntityClassExist(parentFullClassName))
		{
			auto* parentClass = GetEntityClasses().at(parentFullClassName)->_monoClass;
			return IsSubClassOf(child, parentClass, shouldCheckInterface);
		}

		return false;
	}

	bool ScriptEngine::IsSubClassOf(const std::string& childFullClassName, MonoClass* parent, bool shouldCheckInterface)
	{
		if (EntityClassExist(childFullClassName))
		{
			auto* childClass = GetEntityClasses().at(childFullClassName)->_monoClass;
			return IsSubClassOf(childClass, parent, shouldCheckInterface);
		}

		return false;
	}
}
