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

	static std::string ProjectionTypeToString(SceneCamera::ProjectionType projectionType)
	{
		switch (projectionType)
		{
		case SceneCamera::ProjectionType::Perspective: return "Perspective";
		case SceneCamera::ProjectionType::Orthographic: return "Orthographic";
		}

		HZ_CORE_ASSERT(false, "Unknown Projection Type");
		return {};
	}

	static SceneCamera::ProjectionType ProjectionTypeFromString(const std::string& projectionTypeString)
	{
		if (projectionTypeString == "Perspective" || projectionTypeString == "0")
		{
			return SceneCamera::ProjectionType::Perspective;
		}

		if (projectionTypeString == "Orthographic" || projectionTypeString == "1")
		{
			return SceneCamera::ProjectionType::Orthographic;
		}

		HZ_CORE_ASSERT(false, "Unknown Projection Type");
		return SceneCamera::ProjectionType::Perspective;
	}

	static std::string Rigidbody2DBodyTypeToString(Rigidbody2DComponent::BodyType bodyType)
	{
		switch (bodyType)
		{
		case Rigidbody2DComponent::BodyType::Static: return "Static";
		case Rigidbody2DComponent::BodyType::Dynamic: return "Dynamic";
		case Rigidbody2DComponent::BodyType::Kinematic: return "Kinematic";
		}

		HZ_CORE_ASSERT(false, "Unknown Body Type");
		return {};
	}

	static Rigidbody2DComponent::BodyType Rigidbody2DBodyTypeFromString(const std::string& bodyTypeString)
	{
		if (bodyTypeString == "Static") return  Rigidbody2DComponent::BodyType::Static;
		if (bodyTypeString == "Dynamic") return  Rigidbody2DComponent::BodyType::Dynamic;
		if (bodyTypeString == "Kinematic") return  Rigidbody2DComponent::BodyType::Kinematic;

		HZ_CORE_ASSERT(false, "Unknown Body Type");
		return  Rigidbody2DComponent::BodyType::Static;
	}

	static void SerializeEntity(YAML::Emitter& out, Entity entity)
	{
		HZ_CORE_ASSERT(entity.HasComponent<IDComponent>(), "IDComponent missing!");

		out << YAML::BeginMap; // Entity
		out << YAML::Key << "Entity" << YAML::Value << entity.GetUUID();

#pragma region BaseComponent
		out << YAML::Key << "BaseComponent";
		out << YAML::BeginMap; // BaseComponent
		out << YAML::Key << "Name" << YAML::Value << entity.Name();
		out << YAML::Key << "Tag" << YAML::Value << entity.Tag();
		out << YAML::Key << "Layer" << YAML::Value << entity.Layer();
		out << YAML::EndMap; // BaseComponent  
#pragma endregion

#pragma region TransformComponent
		out << YAML::Key << "TransformComponent";
		out << YAML::BeginMap; // TransformComponent
		out << YAML::Key << "Position" << YAML::Value << entity.Transform().Position;
		out << YAML::Key << "Rotation" << YAML::Value << entity.Transform().Rotation;
		out << YAML::Key << "Scale" << YAML::Value << entity.Transform().Scale;
		out << YAML::EndMap; // TransformComponent  
#pragma endregion

#pragma region CameraComponent
		if (const auto& component = entity.TryGetComponent<CameraComponent>())
		{
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap; // CameraComponent

			auto& camera = component->Camera;

			out << YAML::Key << "Camera" << YAML::Value;
			out << YAML::BeginMap; // Camera
			auto v = ProjectionTypeToString(camera.GetProjectionType());
			out << YAML::Key << "ProjectionType" << YAML::Value << ProjectionTypeToString(camera.GetProjectionType());
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
#pragma endregion

#pragma region SpriteRendererComponent
		if (const auto& component = entity.TryGetComponent<SpriteRendererComponent>())
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
#pragma endregion

#pragma region CircleRendererComponent
		if (const auto& component = entity.TryGetComponent<CircleRendererComponent>())
		{
			out << YAML::Key << "CircleRendererComponent";
			out << YAML::BeginMap; // CircleRendererComponent

			out << YAML::Key << "Color" << YAML::Value << component->Color;
			out << YAML::Key << "Thickness" << YAML::Value << component->Thickness;
			out << YAML::Key << "Fade" << YAML::Value << component->Fade;

			out << YAML::EndMap; // CircleRendererComponent
		}
#pragma endregion

#pragma region Rigidbody2DComponent
		if (const auto& component = entity.TryGetComponent<Rigidbody2DComponent>())
		{
			out << YAML::Key << "Rigidbody2DComponent";
			out << YAML::BeginMap; // Rigidbody2DComponent

			out << YAML::Key << "BodyType" << YAML::Value << Rigidbody2DBodyTypeToString(component->Type);
			out << YAML::Key << "IsFixedRotation" << YAML::Value << component->IsFixedRotation;

			out << YAML::EndMap; // Rigidbody2DComponent
		}
#pragma endregion

#pragma region BoxCollider2DComponent
		if (const auto& component = entity.TryGetComponent<BoxCollider2DComponent>())
		{
			out << YAML::Key << "BoxCollider2DComponent";
			out << YAML::BeginMap; // BoxCollider2DComponent

			out << YAML::Key << "Offset" << YAML::Value << component->Offset;
			out << YAML::Key << "Size" << YAML::Value << component->Size;
			out << YAML::Key << "Angle" << YAML::Value << component->Angle;
			out << YAML::Key << "Density" << YAML::Value << component->Density;
			out << YAML::Key << "Friction" << YAML::Value << component->Friction;
			out << YAML::Key << "Restitution" << YAML::Value << component->Restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << component->RestitutionThreshold;

			out << YAML::EndMap; // BoxCollider2DComponent
		}
#pragma endregion

#pragma region CircleCollider2DComponent
		if (const auto& component = entity.TryGetComponent<CircleCollider2DComponent>())
		{
			out << YAML::Key << "CircleCollider2DComponent";
			out << YAML::BeginMap; // CircleCollider2DComponent

			out << YAML::Key << "Offset" << YAML::Value << component->Offset;
			out << YAML::Key << "Radius" << YAML::Value << component->Radius;
			out << YAML::Key << "Density" << YAML::Value << component->Density;
			out << YAML::Key << "Friction" << YAML::Value << component->Friction;
			out << YAML::Key << "Restitution" << YAML::Value << component->Restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << component->RestitutionThreshold;

			out << YAML::EndMap; // CircleCollider2DComponent
		}
#pragma endregion
		out << YAML::EndMap;
	}

	template<typename T>
	static T GetValue(const YAML::Node& node, const std::string& fieldName, T fallbackValue)
	{
		if (const auto& field = node[fieldName])
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

		if (auto entities = data["Entities"])
		{
			for (auto entity : entities)
			{
				auto entityID = entity["Entity"].as<uint64_t>();

#pragma region BaseComponent
				std::string name;
				int tag = 0;
				int layer = 0;
				if (auto baseComponent = entity["BaseComponent"])
				{
					name = baseComponent["Name"].as<std::string>();
					tag = baseComponent["Tag"].as<int>();
					layer = baseComponent["Layer"].as<int>();
				}
#pragma endregion

				if (isWithLog)
				{
					HZ_CORE_LTRACE(" Entity: ID[{0}], Name[{1}],Tag[{2}],Layer[{3}]", entityID, name, tag, layer);
				}
				auto deserializedEntity = _scene->CreateEntityWithUUID(entityID, name, tag, layer);

#pragma region TransformComponent
				if (auto transformComponent = entity["TransformComponent"])
				{
					// Entities always have transform
					auto& component = deserializedEntity.Transform();
					component.Position = transformComponent["Position"].as<glm::vec3>();
					component.Rotation = transformComponent["Rotation"].as<glm::vec3>();
					component.Scale = transformComponent["Scale"].as<glm::vec3>();
				}
#pragma endregion

#pragma region CameraComponent
				if (auto cameraComponent = entity["CameraComponent"])
				{
					auto& component = deserializedEntity.AddComponent<CameraComponent>();
					auto cameraProperties = cameraComponent["Camera"];

					component.Camera.SetAspectRatio(GetValue<float>(cameraProperties, "AspectRatio"));
					component.Camera.SetProjectionType(ProjectionTypeFromString(GetValue<std::string>(cameraProperties, "ProjectionType", "Perspective")));

					component.Camera.SetPerspectiveVerticalFOV(cameraProperties["PerspectiveVerticalFOV"].as<float>());
					component.Camera.SetPerspectiveNearClip(cameraProperties["PerspectiveNearClip"].as<float>());
					component.Camera.SetPerspectiveFarClip(cameraProperties["PerspectiveFarClip"].as<float>());

					component.Camera.SetOrthographicSize(cameraProperties["OrthographicSize"].as<float>());
					component.Camera.SetOrthographicNearClip(cameraProperties["OrthographicNearClip"].as<float>());
					component.Camera.SetOrthographicFarClip(cameraProperties["OrthographicFarClip"].as<float>());

					component.IsPrimary = cameraComponent["IsPrimary"].as<bool>();
					component.IsFixedAspectRatio = cameraComponent["IsFixedAspectRatio"].as<bool>();
				}
#pragma endregion

#pragma region SpriteRendererComponent
				if (auto spriteRendererComponent = entity["SpriteRendererComponent"])
				{
					auto& component = deserializedEntity.AddComponent<SpriteRendererComponent>();

					if (auto texture = spriteRendererComponent["TexturePath"])
					{
						component.Texture = Texture2D::Create(texture.as<std::string>()); // TODO not use path.
						component.Tiling = spriteRendererComponent["Tiling"].as<glm::vec2>();
					}

					component.Color = GetValue<glm::vec4>(spriteRendererComponent, "Color", Color::White);
				}
#pragma endregion

#pragma region CircleRendererComponent
				if (auto circleRendererComponent = entity["CircleRendererComponent"])
				{
					auto& component = deserializedEntity.AddComponent<CircleRendererComponent>();
					component.Color = GetValue<glm::vec4>(circleRendererComponent, "Color", Color::White);
					component.Thickness = GetValue<float>(circleRendererComponent, "Thickness", 1.0f);
					component.Fade = GetValue<float>(circleRendererComponent, "Fade", 0.005f);
				}
#pragma endregion

#pragma region Rigidbody2DComponent
				if (auto rigidbody2DComponent = entity["Rigidbody2DComponent"])
				{
					auto& component = deserializedEntity.AddComponent<Rigidbody2DComponent>();
					component.Type = Rigidbody2DBodyTypeFromString(GetValue<std::string>(rigidbody2DComponent, "BodyType", "Static"));
					component.IsFixedRotation = GetValue<bool>(rigidbody2DComponent, "IsFixedRotation");
				}
#pragma endregion

#pragma region BoxCollider2DComponent
				if (auto boxCollider2DComponent = entity["BoxCollider2DComponent"])
				{
					auto& component = deserializedEntity.AddComponent<BoxCollider2DComponent>();
					component.Offset = GetValue<glm::vec2>(boxCollider2DComponent, "Offset");
					component.Size = GetValue<glm::vec2>(boxCollider2DComponent, "Size", { 0.5f, 0.5f });
					component.Angle = GetValue<float>(boxCollider2DComponent, "Angle", 0.0f);
					component.Density = GetValue<float>(boxCollider2DComponent, "Density", 1.0f);
					component.Friction = GetValue<float>(boxCollider2DComponent, "Friction", 0.5f);
					component.Restitution = GetValue<float>(boxCollider2DComponent, "Restitution");
					component.RestitutionThreshold = GetValue<float>(boxCollider2DComponent, "RestitutionThreshold", 0.5f);
				}
#pragma endregion

#pragma region circleCollider2DComponent
				if (auto circleCollider2DComponent = entity["CircleCollider2DComponent"])
				{
					auto& component = deserializedEntity.AddComponent<CircleCollider2DComponent>();
					component.Offset = GetValue<glm::vec2>(circleCollider2DComponent, "Offset");
					component.Radius = GetValue<float>(circleCollider2DComponent, "Radius", 0.5f);
					component.Density = GetValue<float>(circleCollider2DComponent, "Density", 1.0f);
					component.Friction = GetValue<float>(circleCollider2DComponent, "Friction", 0.5f);
					component.Restitution = GetValue<float>(circleCollider2DComponent, "Restitution");
					component.RestitutionThreshold = GetValue<float>(circleCollider2DComponent, "RestitutionThreshold", 0.5f);
				}
#pragma endregion
			}
		}

		return true;
	}
}
