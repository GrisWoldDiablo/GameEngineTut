#include "hzpch.h"
#include "SceneSerializer.h"

#include "Entity.h"
#include "Components.h"

namespace Hazel
{
	SceneSerializer::SceneSerializer(const Ref<Scene>& scene)
		: _scene(scene)
	{}

	static void SerializeEntity(YAML::Emitter& out, Entity entity)
	{
		out << YAML::BeginMap; // Entity
		out << YAML::Key << "Entity" << YAML::Value << "1657643249657"; // TODO : Entity ID goes here.

		if (auto component = entity.TryGetComponent<TagComponent>(); component != nullptr)
		{
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap; // TagComponent

			out << YAML::Key << "Tag" << YAML::Value << component->Tag;

			out << YAML::EndMap; // TagComponent
		}

		if (auto component = entity.TryGetComponent<TransformComponent>(); component != nullptr)
		{
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap; // TransformComponent

			out << YAML::Key << "Position" << YAML::Value << component->Position;
			out << YAML::Key << "Rotation" << YAML::Value << component->Rotation;
			out << YAML::Key << "Scale" << YAML::Value << component->Scale;

			out << YAML::EndMap; // TransformComponent
		}

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

	void SceneSerializer::Serialize(const std::string& filepath)
	{
		auto fileName = std::string(filepath);
		fileName = fileName.erase(0, fileName.find_last_of('\\') + 1);
		fileName = fileName.erase(fileName.find(".hazel"), fileName.length());
		_scene->SetName(fileName);

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << _scene->_name;
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

		_scene->_registry.each([&](auto entityID)
		{
			Entity entity = { entityID, _scene.get() };
			if (entity == Entity::Null)
			{
				return;
			}

			SerializeEntity(out, entity);
		});

		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(filepath);
		fout << out.c_str();
	}

	void SceneSerializer::SerializeRuntime(const std::string& filepath)
	{
		// Not implemented
		HZ_CORE_ASSERT(false, "Not implemented yet!");
	}

	bool SceneSerializer::Deserialize(const std::string& filepath)
	{
		YAML::Node data = YAML::LoadFile(filepath);
		if (!data["Scene"])
		{
			return false;
		}

		auto sceneName = data["Scene"].as<std::string>();
		HZ_CORE_LTRACE("Deserializing scene name['{0}']", sceneName);
		_scene->SetName(sceneName);

		auto entities = data["Entities"];
		if (entities)
		{
			for (auto entity : entities)
			{
				auto entityID = entity["Entity"].as<uint64_t>(); // TODO Entity ID

				std::string name;
				auto tagComponent = entity["TagComponent"];
				if (tagComponent)
				{
					name = tagComponent["Tag"].as<std::string>();
				}

				HZ_CORE_LTRACE(" Entity: ID['{0}'], name['{1}']", entityID, name);

				auto deserializedEntity = _scene->CreateEntity(name);

				auto transformComponent = entity["TransformComponent"];
				if (transformComponent)
				{
					// Entities always have transform
					auto& component = deserializedEntity.GetComponent<TransformComponent>();
					component.Position = transformComponent["Position"].as<glm::vec3>();
					component.Rotation = transformComponent["Rotation"].as<glm::vec3>();
					component.Scale = transformComponent["Scale"].as<glm::vec3>();
				}

				auto cameraComponent = entity["CameraComponent"];
				if (cameraComponent)
				{
					auto& component = deserializedEntity.AddComponent<CameraComponent>();
					auto& cameraProperties = cameraComponent["Camera"];
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

					component.Color = spriteRendererComponent["Color"].as<glm::vec4>();
				}
			}
		}

		return true;
	}

	bool SceneSerializer::DeserializeRuntime(const std::string& filepath)
	{
		// Not implemented
		HZ_CORE_ASSERT(false, "Not implemented yet!");

		return false;
	}
}
