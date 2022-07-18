#include "hzpch.h"
#include "ScriptEngine.h"

#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"
#include "mono/metadata/object.h"

namespace Hazel
{
	struct StriptEngineData
	{
		MonoDomain* RootDomain = nullptr;
		MonoDomain* AppDomain = nullptr;

		MonoAssembly* CoreAssembly = nullptr;
	};

	static StriptEngineData* sData = nullptr;

	void ScriptEngine::Init()
	{
		sData = new StriptEngineData();

		InitMono();
	}

	void ScriptEngine::Shutdown()
	{
		ShutdownMono();

		delete sData;
	}

	char* ReadBytes(const std::string& filePath, uint32_t* outSize)
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

	MonoAssembly* LoadCSharpAssembly(const std::string& assemblyPath)
	{
		uint32_t fileSize = 0;
		char* fileData = ReadBytes(assemblyPath, &fileSize);
		
		MonoImageOpenStatus status;
		MonoImage* image = mono_image_open_from_data_full(fileData, fileSize, 1, &status, 0);

		if (status != MONO_IMAGE_OK)
		{
			const char* errorMessage = mono_image_strerror(status);
			HZ_CORE_LERROR("Mono Error: {}", errorMessage);
			return nullptr;
		}

		MonoAssembly* assembly = mono_assembly_load_from_full(image, assemblyPath.c_str(), &status, 0);
		mono_image_close(image);

		delete[] fileData;
		return assembly;
	}

	void PrintAssemblyType(MonoAssembly* assembly)
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

	void ScriptEngine::InitMono()
	{
		mono_set_assemblies_path("mono/lib");

		MonoDomain* rootDomain = mono_jit_init("HazelJITRuntime");
		HZ_CORE_ASSERT(rootDomain, "MonoDomain could not be initialized!");

		// Store the root Domain
		sData->RootDomain = rootDomain;

		// Create an App Domain
		sData->AppDomain = mono_domain_create_appdomain("HazelScriptRuntime", nullptr);
		mono_domain_set(sData->AppDomain, true);

		// Print assembly
		sData->CoreAssembly = LoadCSharpAssembly("Resources/Scripts/Hazel-ScriptCore.dll");
		if (!sData->CoreAssembly)
		{
			return;
		}

		PrintAssemblyType(sData->CoreAssembly);

		// 0. Get the assembly 
		MonoImage* assemblyImage = mono_assembly_get_image(sData->CoreAssembly);

		// 1. Create an object (and call constructor)
		MonoClass* monoClass = mono_class_from_name(assemblyImage, "Hazel", "Main");
		MonoObject* instance = mono_object_new(sData->AppDomain, monoClass);
		mono_runtime_object_init(instance);

		// 2. Call function
		MonoMethod* printMessageFunc = mono_class_get_method_from_name(monoClass, "PrintMessage", 0);
		mono_runtime_invoke(printMessageFunc, instance, nullptr, nullptr);

		// 3. Call function with parameter
		// Single param, only works if there is not different signature of the method (different param type)
		MonoMethod* printMessageFuncParam = mono_class_get_method_from_name(monoClass, "PrintMessage", 1);
		int value = 5;
		void* param = &value;
		mono_runtime_invoke(printMessageFuncParam, instance, &param, nullptr);

		// Multiple param
		MonoMethod* printMessageFuncParams = mono_class_get_method_from_name(monoClass, "PrintMessage", 2);
		int value1 = 69;
		int value2 = 420;
		void* params[2] =
		{
			&value1,
			&value2
		};
		mono_runtime_invoke(printMessageFuncParams, instance, params, nullptr);

		// String param
		MonoMethod* printCustomMessageFunc = mono_class_get_method_from_name(monoClass, "PrintCustomMessage", 1);
		MonoString* monoString = mono_string_new(sData->AppDomain, "Hello World from C++!");
		void* paramString = monoString;
		mono_runtime_invoke(printCustomMessageFunc, instance, &paramString, nullptr);

		//HZ_CORE_ASSERT(false, "");
	}

	void ScriptEngine::ShutdownMono()
	{
		// TODO: unload fails, find solution.
		//mono_domain_unload(sData->AppDomain);
		sData->AppDomain = nullptr;

		mono_jit_cleanup(sData->RootDomain);
		sData->RootDomain = nullptr;
	}
}
