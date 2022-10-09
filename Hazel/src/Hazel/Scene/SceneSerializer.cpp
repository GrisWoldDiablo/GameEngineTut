#include "hzpch.h"
#include "SceneSerializer.h"

#include "Entity.h"
#include "Components.h"

#include "Hazel/Scripting/ScriptEngine.h"
#include "Hazel/Scripting/ScriptClass.h"

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
		if (entity.HasComponent<CameraComponent>())
		{
			const auto& component = entity.GetComponent<CameraComponent>();

			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap; // CameraComponent

			auto& camera = component.Camera;

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

			out << YAML::Key << "IsPrimary" << YAML::Value << component.IsPrimary;
			out << YAML::Key << "IsFixedAspectRatio" << YAML::Value << component.IsFixedAspectRatio;

			out << YAML::EndMap; // CameraComponent
		}
#pragma endregion

#pragma region ScriptComponent
		if (entity.HasComponent<ScriptComponent>())
		{
			const auto& component = entity.GetComponent<ScriptComponent>();

			out << YAML::Key << "ScriptComponent";
			out << YAML::BeginMap; // ScriptComponent

			out << YAML::Key << "ClassName" << YAML::Value << component.ClassName;

			// Fields
			auto entityClass = ScriptEngine::GetEntityClass(component.ClassName);
			const auto& fields = entityClass->GetFields();

			if (fields.size() > 0)
			{
				out << YAML::Key << "ScriptFields" << YAML::Value;

				auto& entityFields = ScriptEngine::GetScriptFieldMap(entity);

				out << YAML::BeginSeq; // ScriptFields Sequence

				for (const auto& [name, field] : fields)
				{
					if (entityFields.find(name) == entityFields.end())
					{
						continue;
					}

					out << YAML::BeginMap; // Field Map

					out << YAML::Key << "Name" << YAML::Value << name;
					out << YAML::Key << "Type" << YAML::Value << field.Type;

					const auto& scriptField = entityFields.at(name);

					out << YAML::Key << "Data" << YAML::Value;

					switch (field.Type)
					{
					case ScriptFieldType::Float:
					{
						out << scriptField.GetValue<float>();
						break;
					}
					case ScriptFieldType::Double:
					{
						out << scriptField.GetValue<double>();
						break;
					}
					case ScriptFieldType::Char:
					{
						out << scriptField.GetValue<uint8_t>();
						break;
					}
					case ScriptFieldType::Bool:
					{
						out << scriptField.GetValue<bool>();
						break;
					}
					case ScriptFieldType::SByte:
					{
						out << scriptField.GetValue<int8_t>();
						break;
					}
					case ScriptFieldType::Short:
					{
						out << scriptField.GetValue<int16_t>();
						break;
					}
					case ScriptFieldType::Int:
					{
						out << scriptField.GetValue<int32_t>();
						break;
					}
					case ScriptFieldType::Long:
					{
						out << scriptField.GetValue<int64_t>();
						break;
					}
					case ScriptFieldType::Byte:
					{
						out << scriptField.GetValue<uint16_t>();
						break;
					}
					case ScriptFieldType::UShort:
					{
						out << scriptField.GetValue<uint16_t>();
						break;
					}
					case ScriptFieldType::UInt:
					{
						out << scriptField.GetValue<uint32_t>();
						break;
					}
					case ScriptFieldType::ULong:
					{
						out << scriptField.GetValue<uint64_t>();
						break;
					}
					case ScriptFieldType::Vector2:
					{
						out << scriptField.GetValue<glm::vec2>();
						break;
					}
					case ScriptFieldType::Vector3:
					{
						out << scriptField.GetValue<glm::vec3>();
						break;
					}
					case ScriptFieldType::Vector4:
					{
						out << scriptField.GetValue<glm::vec4>();
						break;
					}
					case ScriptFieldType::Color:
					{
						out << scriptField.GetValue<Color>();
						break;
					}
					case ScriptFieldType::Entity:
					{
						out << scriptField.GetValue<UUID>();
						break;
					}
					case ScriptFieldType::String:
					{
						out << scriptField.GetStringValue();
						break;
					}
					case ScriptFieldType::None:
					default:
					{
						out << "None";
						break;
					}
					}

					out << YAML::EndMap; // Field Map
				}

				out << YAML::EndSeq; // ScriptFields Sequence 
			}

			out << YAML::EndMap; // ScriptComponent  
		}
#pragma endregion

#pragma region SpriteRendererComponent
		if (entity.HasComponent<SpriteRendererComponent>())
		{
			const auto& component = entity.GetComponent<SpriteRendererComponent>();

			out << YAML::Key << "SpriteRendererComponent";
			out << YAML::BeginMap; // SpriteRendererComponent

			if (component.Texture != nullptr)
			{
				out << YAML::Key << "TexturePath" << YAML::Value << component.Texture->GetPath(); // TODO not use path but actual texture asset.
				out << YAML::Key << "MagFilter" << YAML::Value << component.Texture->GetMagFilter();
			}

			out << YAML::Key << "Tiling" << YAML::Value << component.Tiling;
			out << YAML::Key << "Color" << YAML::Value << component.Color;

			out << YAML::EndMap; // SpriteRendererComponent
		}
#pragma endregion

#pragma region CircleRendererComponent
		if (entity.HasComponent<CircleRendererComponent>())
		{
			const auto& component = entity.GetComponent<CircleRendererComponent>();

			out << YAML::Key << "CircleRendererComponent";
			out << YAML::BeginMap; // CircleRendererComponent

			out << YAML::Key << "Color" << YAML::Value << component.Color;
			out << YAML::Key << "Thickness" << YAML::Value << component.Thickness;
			out << YAML::Key << "Fade" << YAML::Value << component.Fade;

			out << YAML::EndMap; // CircleRendererComponent
		}
#pragma endregion

#pragma region Rigidbody2DComponent
		if (entity.HasComponent<Rigidbody2DComponent>())
		{
			const auto& component = entity.GetComponent<Rigidbody2DComponent>();

			out << YAML::Key << "Rigidbody2DComponent";
			out << YAML::BeginMap; // Rigidbody2DComponent

			out << YAML::Key << "BodyType" << YAML::Value << Rigidbody2DBodyTypeToString(component.Type);
			out << YAML::Key << "IsFixedRotation" << YAML::Value << component.IsFixedRotation;

			out << YAML::EndMap; // Rigidbody2DComponent
		}
#pragma endregion

#pragma region BoxCollider2DComponent
		if (entity.HasComponent<BoxCollider2DComponent>())
		{
			const auto& component = entity.GetComponent<BoxCollider2DComponent>();

			out << YAML::Key << "BoxCollider2DComponent";
			out << YAML::BeginMap; // BoxCollider2DComponent

			out << YAML::Key << "Offset" << YAML::Value << component.Offset;
			out << YAML::Key << "Size" << YAML::Value << component.Size;
			out << YAML::Key << "Rotation" << YAML::Value << component.Rotation;
			out << YAML::Key << "Density" << YAML::Value << component.Density;
			out << YAML::Key << "Friction" << YAML::Value << component.Friction;
			out << YAML::Key << "Restitution" << YAML::Value << component.Restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << component.RestitutionThreshold;

			out << YAML::EndMap; // BoxCollider2DComponent
		}
#pragma endregion

#pragma region CircleCollider2DComponent
		if (entity.HasComponent<CircleCollider2DComponent>())
		{
			const auto& component = entity.GetComponent<CircleCollider2DComponent>();

			out << YAML::Key << "CircleCollider2DComponent";
			out << YAML::BeginMap; // CircleCollider2DComponent

			out << YAML::Key << "Offset" << YAML::Value << component.Offset;
			out << YAML::Key << "Radius" << YAML::Value << component.Radius;
			out << YAML::Key << "Density" << YAML::Value << component.Density;
			out << YAML::Key << "Friction" << YAML::Value << component.Friction;
			out << YAML::Key << "Restitution" << YAML::Value << component.Restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << component.RestitutionThreshold;

			out << YAML::EndMap; // CircleCollider2DComponent
		}
#pragma endregion

#pragma region AudioSourceComponent
		if (entity.HasComponent<AudioSourceComponent>())
		{
			const auto& component = entity.GetComponent<AudioSourceComponent>();

			out << YAML::Key << "AudioSourceComponent";
			out << YAML::BeginMap; // AudioSourceComponent

			if (component.AudioSource != nullptr)
			{
				out << YAML::Key << "AudioClipPath" << YAML::Value << component.AudioSource->GetPath(); // TODO not use path but actual texture asset.
				out << YAML::Key << "Gain" << YAML::Value << component.AudioSource->GetGain();
				out << YAML::Key << "Pitch" << YAML::Value << component.AudioSource->GetPitch();
				out << YAML::Key << "IsLoop" << YAML::Value << component.AudioSource->GetLoop();
				out << YAML::Key << "Is3D" << YAML::Value << component.AudioSource->Get3D();
			}

			out << YAML::Key << "IsVisibleInGame" << YAML::Value << component.IsVisibleInGame;

			out << YAML::EndMap; // AudioSourceComponent
		}
#pragma endregion

#pragma region AudioListenerComponent
		if (entity.HasComponent<AudioListenerComponent>())
		{
			const auto& component = entity.GetComponent<AudioListenerComponent>();

			out << YAML::Key << "AudioListenerComponent";
			out << YAML::BeginMap; // AudioListenerComponent

			out << YAML::Key << "IsVisibleInGame" << YAML::Value << component.IsVisibleInGame;

			out << YAML::EndMap; // AudioListenerComponent
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

	void SceneSerializer::Serialize(const std::filesystem::path& filepath)
	{
		_scene->SetName(filepath.filename().replace_extension().string());

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

	bool SceneSerializer::Deserialize(const std::filesystem::path& filepath, bool isWithLog)
	{
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filepath.string());
		}
		catch (YAML::ParserException& e)
		{
			HZ_CORE_LERROR("Failed to load file [{0}].\n\t'{1}'", filepath, e.what());
			return false;
		}

		return DeserializeData(data, isWithLog);
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

#pragma region ScriptComponent
				if (auto scriptComponent = entity["ScriptComponent"])
				{
					auto& component = deserializedEntity.AddComponent<ScriptComponent>();
					component.ClassName = GetValue<std::string>(scriptComponent, "ClassName");

					if (auto scriptFields = scriptComponent["ScriptFields"])
					{
						if (const auto& entityClass = ScriptEngine::GetEntityClass(component.ClassName))
						{
							auto& entityFields = ScriptEngine::GetScriptFieldMap(deserializedEntity);
							const auto& fields = entityClass->GetFields();

							for (auto scriptField : scriptFields)
							{
								auto name = GetValue<std::string>(scriptField, "Name");

								if (fields.find(name) == fields.end())
								{
									HZ_CORE_LWARN("Deserialization: Field [{0}] doesn't not exist on class [{1}]", name, component.ClassName);
									continue;
								}

								auto type = GetValue<ScriptFieldType>(scriptField, "Type");
								auto& scriptFieldInstance = entityFields[name];
								scriptFieldInstance.Field = fields.at(name);

								switch (type)
								{
								case ScriptFieldType::Float:
								{
									auto data = GetValue<float>(scriptField, "Data");
									scriptFieldInstance.SetValue(data);
									break;
								}
								case ScriptFieldType::Double:
								{
									auto data = GetValue<double>(scriptField, "Data");
									scriptFieldInstance.SetValue(data);
									break;
								}
								case ScriptFieldType::Char:
								{
									auto data = static_cast<uint16_t>(GetValue<char>(scriptField, "Data"));
									scriptFieldInstance.SetValue(data);
									break;
								}
								case ScriptFieldType::Bool:
								{
									auto data = GetValue<bool>(scriptField, "Data");
									scriptFieldInstance.SetValue(data);
									break;
								}
								case ScriptFieldType::SByte:
								{
									auto data = GetValue<int8_t>(scriptField, "Data");
									scriptFieldInstance.SetValue(data);
									break;
								}
								case ScriptFieldType::Short:
								{
									auto data = GetValue<int16_t>(scriptField, "Data");
									scriptFieldInstance.SetValue(data);
									break;
								}
								case ScriptFieldType::Int:
								{
									auto data = GetValue<int32_t>(scriptField, "Data");
									scriptFieldInstance.SetValue(data);
									break;
								}
								case ScriptFieldType::Long:
								{
									auto data = GetValue<int64_t>(scriptField, "Data");
									scriptFieldInstance.SetValue(data);
									break;
								}
								case ScriptFieldType::Byte:
								{
									auto data = GetValue<uint8_t>(scriptField, "Data");
									scriptFieldInstance.SetValue(data);
									break;
								}
								case ScriptFieldType::UShort:
								{
									auto data = GetValue<uint16_t>(scriptField, "Data");
									scriptFieldInstance.SetValue(data);
									break;
								}
								case ScriptFieldType::UInt:
								{
									auto data = GetValue<uint32_t>(scriptField, "Data");
									scriptFieldInstance.SetValue(data);
									break;
								}
								case ScriptFieldType::ULong:
								{
									auto data = GetValue<uint64_t>(scriptField, "Data");
									scriptFieldInstance.SetValue(data);
									break;
								}
								case ScriptFieldType::Vector2:
								{
									auto data = GetValue<glm::vec2>(scriptField, "Data");
									scriptFieldInstance.SetValue(data);
									break;
								}
								case ScriptFieldType::Vector3:
								{
									auto data = GetValue<glm::vec3>(scriptField, "Data");
									scriptFieldInstance.SetValue(data);
									break;
								}
								case ScriptFieldType::Vector4:
								{
									auto data = GetValue<glm::vec4>(scriptField, "Data");
									scriptFieldInstance.SetValue(data);
									break;
								}
								case ScriptFieldType::Color:
								{
									auto data = GetValue<glm::vec4>(scriptField, "Data");
									scriptFieldInstance.SetValue(data);
									break;
								}
								case ScriptFieldType::Entity:
								{
									auto data = GetValue<uint64_t>(scriptField, "Data");
									scriptFieldInstance.SetValue(data);
									break;
								}
								case ScriptFieldType::String:
								{
									auto data = GetValue<std::string>(scriptField, "Data");
									scriptFieldInstance.SetStringValue(data);
									break;
								}
								case ScriptFieldType::None:
								default:
								{
									auto data = "";
									scriptFieldInstance.SetValue(data);
									break;
								}
								}
							}
						}
					}
				}
#pragma endregion

#pragma region SpriteRendererComponent
				if (auto spriteRendererComponent = entity["SpriteRendererComponent"])
				{
					auto& component = deserializedEntity.AddComponent<SpriteRendererComponent>();

					if (auto texturePath = spriteRendererComponent["TexturePath"])
					{
						component.Texture = Texture2D::Create(texturePath.as<std::filesystem::path>()); // TODO not use path use asset.
						if (component.Texture != nullptr)
						{
							component.Texture->SetMagFilter(GetValue<uint32_t>(spriteRendererComponent, "MagFilter", 0x2601));
						}
					}

					component.Tiling = GetValue<glm::vec2>(spriteRendererComponent, "Tiling", { 1.0f,1.0f });
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
					component.Rotation = GetValue<float>(boxCollider2DComponent, "Rotation", 0.0f);
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

#pragma region AudioSourceComponent
				if (auto audioSourceComponent = entity["AudioSourceComponent"])
				{
					auto& component = deserializedEntity.AddComponent<AudioSourceComponent>();

					if (auto audioClipPath = audioSourceComponent["AudioClipPath"])
					{
						component.AudioSource = AudioSource::Create(audioClipPath.as<std::filesystem::path>()); // TODO not use path use asset.
						if (component.AudioSource != nullptr)
						{
							component.AudioSource->SetGain(GetValue<float>(audioSourceComponent, "Gain", 1.0f));
							component.AudioSource->SetPitch(GetValue<float>(audioSourceComponent, "Pitch", 1.0f));
							component.AudioSource->SetLoop(GetValue<bool>(audioSourceComponent, "IsLoop", false));
							component.AudioSource->Set3D(GetValue<bool>(audioSourceComponent, "Is3D", false));
							component.AudioSource->SetPosition(deserializedEntity.Transform().Position);
						}
					}

					component.IsVisibleInGame = GetValue<bool>(audioSourceComponent, "IsVisibleInGame", false);
				}
#pragma endregion

#pragma region AudioListenerComponent
				if (auto audioListenerComponent = entity["AudioListenerComponent"])
				{
					auto& component = deserializedEntity.AddComponent<AudioListenerComponent>();
					component.IsVisibleInGame = GetValue<bool>(audioListenerComponent, "IsVisibleInGame", false);
				}
#pragma endregion
			}
		}

		return true;
	}
}
