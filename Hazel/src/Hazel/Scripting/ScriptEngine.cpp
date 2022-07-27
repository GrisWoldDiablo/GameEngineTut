#include "hzpch.h"
#include "ScriptEngine.h"
#include "ScriptGlue.h"
#include "Hazel/Scene/Scene.h"

#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"
#include "mono/metadata/object.h"

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

		ScriptClass EntityClass;

		std::unordered_map<std::string, Ref<ScriptClass>> EntityClasses;

		// Runtime
		Scene* SceneContext = nullptr;
	};

	static StriptEngineData* sData = nullptr;

	///////////////////////////
	/// SCRIPT ENGINE
	///////////////////////////

#define ASSEMBLY_PATH "Resources/Scripts/Hazel-ScriptCore.dll"

	void ScriptEngine::Init()
	{
		sData = new StriptEngineData();

		InitMono();

		if (!TryLoadCSharpAssembly(ASSEMBLY_PATH))
		{
			return;
		}

		LoadAssemblyClasses(sData->CoreAssembly);

		ScriptGlue::RegisterFunctions();

		DemoFunctionality();
	}

	void ScriptEngine::Shutdown()
	{
		ShutdownMono();

		delete sData;
	}

	bool ScriptEngine::TryLoadCSharpAssembly(const std::filesystem::path& filePath)
	{
		constexpr char* domainName = "HazelScriptRuntime";
		MonoDomain* appDomain = mono_domain_create_appdomain(domainName, nullptr);
		if (mono_domain_set(appDomain, false))
		{
			sData->AppDomain = appDomain;

			if (MonoAssembly* coreAssembly = Utils::LoadMonoAssembly(filePath))
			{
				sData->CoreAssembly = coreAssembly;

				sData->CoreAssemblyImage = mono_assembly_get_image(sData->CoreAssembly);

				Utils::PrintAssemblyType(sData->CoreAssembly);

				return true;
			}

			HZ_CORE_LCRITICAL("Failed to load assembly {0}", filePath);
			return false;
		}

		HZ_CORE_LCRITICAL("Failed to set domain {0}", domainName);
		return false;
	}

	void ScriptEngine::OnRuntimeStart(Scene* scene)
	{
		sData->SceneContext = scene;
	}

	void ScriptEngine::OnRuntimeStop()
	{
		sData->SceneContext = nullptr;
	}

	bool ScriptEngine::TryReload()
	{
		HZ_CORE_LINFO("Reloading ScriptEngine");

		mono_domain_set(mono_get_root_domain(), false);

		MonoObject* exc = NULL;
		mono_domain_try_unload(sData->AppDomain, &exc);
		if (exc)
		{
			return false;
		}


		if (!TryLoadCSharpAssembly(ASSEMBLY_PATH))
		{
			HZ_CORE_LERROR("Fail to reload assembly");
			return false;
		}

		LoadAssemblyClasses(sData->CoreAssembly);

		ScriptGlue::RegisterFunctions();

		HZ_CORE_LINFO("ScriptEngine Reloaded");

		DemoFunctionality();

		return true;
	}

	bool ScriptEngine::EntityClassExist(const std::string& fullClassName)
	{
		return sData->EntityClasses.find(fullClassName) != sData->EntityClasses.end();
	}

	std::unordered_map<std::string, Ref<ScriptClass>> ScriptEngine::GetEntityClasses()
	{
		return sData->EntityClasses;
	}

	void ScriptEngine::InitMono()
	{
		mono_set_assemblies_path("mono/lib");

		MonoDomain* rootDomain = mono_jit_init("HazelJITRuntime");
		HZ_CORE_ASSERT(rootDomain, "MonoDomain could not be initialized!");

		// Store the root Domain
		sData->RootDomain = rootDomain;
	}

	void ScriptEngine::ShutdownMono()
	{
		// Since we are unloading the current domain we need to set one before.
		mono_domain_set(mono_get_root_domain(), false);

		mono_domain_unload(sData->AppDomain);
		sData->AppDomain = nullptr;

		mono_jit_cleanup(sData->RootDomain);
		sData->RootDomain = nullptr;
	}

	void ScriptEngine::DemoFunctionality()
	{
		//MonoObject* instance = sData->EntityClass.Instanciate();

		//// 2. Call function
		//MonoMethod* onCreateFunc = sData->EntityClass.GetMethod("OnCreate", 0);
		//sData->EntityClass.InvokeMethod(instance, onCreateFunc);
		
#if 0
		// 1. Create an object (and call constructor)
		sData->EntityClass = ScriptClass("Hazel", "Entity");

		MonoObject* instance = sData->EntityClass.Instanciate();

		// 2. Call function
		MonoMethod* printMessageFunc = sData->EntityClass.GetMethod("PrintMessage", 0);
		sData->EntityClass.InvokeMethod(instance, printMessageFunc);

		// 3. Call function with parameter
		// 3.1 Single param, only works if there is not different signature of the method (different param type)
		MonoMethod* printMessageFuncParam = sData->EntityClass.GetMethod("PrintMessage", 1);
		int value = 5;
		void* param = &value;
		sData->EntityClass.InvokeMethod(instance, printMessageFuncParam, &param);

		// 3.2 Multiple param
		MonoMethod* printMessageFuncParams = sData->EntityClass.GetMethod("PrintMessage", 2);
		int value1 = 69;
		int value2 = 420;
		void* params[2] =
		{
			&value1,
			&value2
		};
		sData->EntityClass.InvokeMethod(instance, printMessageFuncParams, params);

		// 3.3 String param
		MonoMethod* printCustomMessageFunc = sData->EntityClass.GetMethod("PrintCustomMessage", 1);
		MonoString* monoString = mono_string_new(sData->AppDomain, "Hello World from C++!");
		void* paramString = monoString;
		sData->EntityClass.InvokeMethod(instance, printCustomMessageFunc, &paramString);
#endif // 0

	}

	MonoObject* ScriptEngine::InstanciateClass(MonoClass* monoClass)
	{
		MonoObject* monoObject = mono_object_new(sData->AppDomain, monoClass);
		mono_runtime_object_init(monoObject);

		return monoObject;
	}

	void ScriptEngine::LoadAssemblyClasses(MonoAssembly* assembly)
	{
		sData->EntityClasses.clear();

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
				sData->EntityClasses[fullName] = CreateRef<ScriptClass>(nameSpace, name);
			}
			
		}
	}

	///////////////////////////
	/// SCRIPT CLASS
	///////////////////////////

	ScriptClass::ScriptClass(const std::string& classNamespace, const std::string& className)
		:_classNamespace(classNamespace), _className(className)
	{
		_monoClass = mono_class_from_name(sData->CoreAssemblyImage, classNamespace.c_str(), className.c_str());
	}

	MonoObject* ScriptClass::Instanciate()
	{
		return ScriptEngine::InstanciateClass(_monoClass);
	}

	MonoMethod* ScriptClass::GetMethod(const std::string& name, int parameterCount)
	{
		return mono_class_get_method_from_name(_monoClass, name.c_str(), parameterCount);
	}

	MonoObject* ScriptClass::InvokeMethod(MonoObject* instance, MonoMethod* monoMethod, void** params)
	{
		return mono_runtime_invoke(monoMethod, instance, params, nullptr);

	}

	///////////////////////////
	/// SCRIPT INSTANCE
	///////////////////////////

	ScriptInstance::ScriptInstance(Ref<ScriptClass> scriptClass)
		:_scriptClass(scriptClass)
	{
		_instance = scriptClass->Instanciate();
		_onCreateMethod = scriptClass->GetMethod("OnCreate", 0);
		_onUpdateMethod = scriptClass->GetMethod("OnUpdate", 1);
	}

	void ScriptInstance::InvokeOnCreate()
	{
		_scriptClass->InvokeMethod(_instance, _onCreateMethod);
	}

	void ScriptInstance::InvokeOnUpdate(float ts)
	{
		void* param = &ts;
		_scriptClass->InvokeMethod(_instance, _onUpdateMethod, &param);
	}
}
