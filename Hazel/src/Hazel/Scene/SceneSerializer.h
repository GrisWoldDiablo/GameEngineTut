#pragma once
#include "Hazel/Utils/Serializer.h"
#include "Scene.h"

namespace Hazel
{

	class SceneSerializer : public Serializer
	{
	public:
		SceneSerializer(const Ref<Scene>& scene);

		void Serialize(const std::string& filepath) override;
		void SerializeRuntime() override;
		bool Deserialize(const std::string& filepath) override;
		bool DeserializeRuntime() override;

	private:
		void SerializeData(YAML::Emitter& out);
		bool DeserializeData(const YAML::Node& data, bool isWithLog = true);

	private:
		Ref<Scene> _scene;
	};

	static void SerializeEntity(YAML::Emitter& out, Entity entity);
	template<typename T>
	static T GetValue(const YAML::Node& node, const std::string& fieldName, T fallbackValue = T());
}
