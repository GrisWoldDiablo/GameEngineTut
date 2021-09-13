#include "hzpch.h"
#include "SceneSerializer.h"

#include "Entity.h"
#include "Components.h"

namespace Hazel
{
	static std::string _runtimeSceneData;

	SceneSerializer::SceneSerializer(const Ref<Scene>& scene)
		: _scene(scene)
	{}

	static void SerializeEntity(YAML::Emitter& out, Entity entity)
	{
		out << YAML::BeginMap; // Entity
		out << YAML::Key << "Entity" << YAML::Value << "1657643249657"; // TODO : Entity ID goes here.

		out << YAML::Key << "BaseComponent";
		out << YAML::BeginMap; // BaseComponent
		out << YAML::Key << "Name" << YAML::Value << entity.Name();
		out << YAML::Key << "Tag" << YAML::Value << entity.Tag();
		out << YAML::Key << "Layer" << YAML::Value << entity.Layer();
		out << YAML::EndMap; // BaseComponent

		out << YAML::Key << "TransformComponent";
		out << YAML::BeginMap; // TransformComponent
		out << YAML::Key << "Position" << YAML::Value << entity.Transform().Position;
		out << YAML::Key << "Rotation" << YAML::Value << entity.Transform().Rotation;
		out << YAML::Key << "Scale" << YAML::Value << entity.Transform().Scale;
		out << YAML::EndMap; // TransformComponent

		if (auto component = entity.TryGetComponent<CameraComponent>(); component != nullptr)
		{
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap; // CameraComponent

			auto& camera = component->Camera;

			out << YAML::Key << "Camera" << YAML::Value;
			out << YAML::BeginMap; // Camera
			out << YAML::Key << "ProjectionType" << YAML::Value << (int)camera.GetProjectionType();
			out << YAML::Key << "PerspectiveVerticalFOV" << YAML::Value << camera.GetPerspectiveVerticalFOV();
			out << YAML::Key << "PerspectiveNearClip" << YAML::Value << camera.GetPerspectiveNearClip();
			out << YAML::Key << "PerspectiveFarClip" << YAML::Value << camera.GetPerspectiveFarClip();
			out << YAML::Key << "OrthographicSize" << YAML::Value << camera.GetOrthographicSize();
			out << YAML::Key << "OrthographicNearClip" << YAML::Value << camera.GetOrthographicNearClip();
			out << YAML::Key << "OrthographicFarClip" << YAML::Value << camera.GetOrthographicFarClip();
			out << YAML::Key << "AspectRatio" << YAML::Value << camera.GetAspectRatio();
			out << YAML::EndMap; // Camera

			out << YAML::Key << "IsPrimary" << YAML::Value << component->IsPrimary;
			out << YAML::Key << "IsFixedAspectRatio" << YAML::Value << component->IsFixedAspectRatio;

			out << YAML::EndMap; // CameraComponent
		}

		if (auto component = entity.TryGetComponent<SpriteRendererComponent>(); component != nullptr)
		{
			out << YAML::Key << "SpriteRendererComponent";
			out << YAML::BeginMap; // SpriteRendererComponent

			out << YAML::Key << "Color" << YAML::Value << component->Color;
			if (component->Texture != nullptr)
			{
				out << YAML::Key << "TexturePath" << YAML::Value << component->Texture->GetPath(); // TODO not use path but actual texture asset.
				out << YAML::Key << "Tiling" << YAML::Value << component->Tiling;
			}

			out << YAML::EndMap; // SpriteRendererComponent
		}

		out << YAML::EndMap;
	}

	template<typename T>
	static T GetValue(const YAML::Node& node, const std::string& fieldName, T fallbackValue)
	{
		if (auto field = node[fieldName]; field)
		{
			return field.as<T>();
		}

		return fallbackValue;
	}

	void SceneSerializer::Serialize(const std::string& filepath)
	{
		auto fileName = std::string(filepath);
		fileName = fileName.erase(0, fileName.find_last_of('\\') + 1);
		fileName = fileName.erase(fileName.find(".hazel"), fileName.length());
		_scene->SetName(fileName);

		YAML::Emitter out;
		SerializeData(out);

		std::ofstream fout(filepath);
		fout << out.c_str();
	}

	void SceneSerializer::SerializeRuntime()
	{
		YAML::Emitter out;
		SerializeData(out);

		std::stringstream stream;
		stream << out.c_str();
		_runtimeSceneData = stream.str();
	}

	void SceneSerializer::SerializeData(YAML::Emitter& out)
	{
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << _scene->_name;
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

		std::vector<Entity> entities = std::vector<Entity>();
		_scene->_registry.each([&](auto entityID)
		{
			Entity entity = { entityID, _scene.get() };
			if (!entity)
			{
				return;
			}
			entities.push_back(entity);
		});

		std::for_each(entities.rbegin(), entities.rend(), [&](auto entity)
		{
			SerializeEntity(out, entity);
		});

		out << YAML::EndSeq;
		out << YAML::EndMap;
	}

	bool SceneSerializer::Deserialize(const std::string& filepath)
	{
		YAML::Node data = YAML::LoadFile(filepath);
		return DeserializeData(data);
	}

	bool SceneSerializer::DeserializeRuntime()
	{
		YAML::Node data = YAML::Load(_runtimeSceneData.c_str());
		return DeserializeData(data, false);
	}

	bool SceneSerializer::DeserializeData(const YAML::Node& data, bool isWithLog)
	{
		if (!data["Scene"])
		{
			return false;
		}

		auto sceneName = data["Scene"].as<std::string>();
		if (isWithLog)
		{
			HZ_CORE_LTRACE("Deserializing scene name[{0}]", sceneName);
		}
		_scene->SetName(sceneName);

		auto entities = data["Entities"];
		if (entities)
		{
			for (auto entity : entities)
			{
				auto entityID = entity["Entity"].as<uint64_t>(); // TODO Entity ID

				std::string name;
				int tag;
				int layer;
				auto baseComponent = entity["BaseComponent"];
				if (baseComponent)
				{
					name = baseComponent["Name"].as<std::string>();
					tag = baseComponent["Tag"].as<int>();
					layer = baseComponent["Layer"].as<int>();
				}

				if (isWithLog)
				{
					HZ_CORE_LTRACE(" Entity: ID[{0}], Name[{1}],Tag[{2}],Layer[{3}]", entityID, name, tag, layer);
				}
				auto deserializedEntity = _scene->CreateEntity(name, tag, layer);

				auto transformComponent = entity["TransformComponent"];
				if (transformComponent)
				{
					// Entities always have transform
					auto& component = deserializedEntity.Transform();
					component.Position = transformComponent["Position"].as<glm::vec3>();
					component.Rotation = transformComponent["Rotation"].as<glm::vec3>();
					component.Scale = transformComponent["Scale"].as<glm::vec3>();
				}

				auto cameraComponent = entity["CameraComponent"];
				if (cameraComponent)
				{
					auto& component = deserializedEntity.AddComponent<CameraComponent>();
					auto& cameraProperties = cameraComponent["Camera"];

					component.Camera.SetAspectRatio(GetValue<float>(cameraProperties, "AspectRatio"));
					component.Camera.SetProjectionType((SceneCamera::ProjectionType)cameraProperties["ProjectionType"].as<int>());

					component.Camera.SetPerspectiveVerticalFOV(cameraProperties["PerspectiveVerticalFOV"].as<float>());
					component.Camera.SetPerspectiveNearClip(cameraProperties["PerspectiveNearClip"].as<float>());
					component.Camera.SetPerspectiveFarClip(cameraProperties["PerspectiveFarClip"].as<float>());

					component.Camera.SetOrthographicSize(cameraProperties["OrthographicSize"].as<float>());
					component.Camera.SetOrthographicNearClip(cameraProperties["OrthographicNearClip"].as<float>());
					component.Camera.SetOrthographicFarClip(cameraProperties["OrthographicFarClip"].as<float>());

					component.IsPrimary = cameraComponent["IsPrimary"].as<bool>();
					component.IsFixedAspectRatio = cameraComponent["IsFixedAspectRatio"].as<bool>();
				}

				auto spriteRendererComponent = entity["SpriteRendererComponent"];
				if (spriteRendererComponent)
				{
					auto& component = deserializedEntity.AddComponent<SpriteRendererComponent>();
					auto texture = spriteRendererComponent["TexturePath"];

					if (texture)
					{
						component.Texture = Texture2D::Create(texture.as<std::string>()); // TODO not use path.
						component.Tiling = spriteRendererComponent["Tiling"].as<glm::vec2>();
					}

					component.Color = GetValue<glm::vec4>(spriteRendererComponent, "Color", { 0.0f, 0.0f, 0.0f, 1.0f });
				}
			}
		}

		return true;
	}
}
