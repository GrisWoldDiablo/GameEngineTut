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
		void SerializeRuntime(const std::string& filepath) override;

		bool Deserialize(const std::string& filepath) override;
		bool DeserializeRuntime(const std::string& filepath) override;

	private:
		Ref<Scene> _scene;
	};
}
