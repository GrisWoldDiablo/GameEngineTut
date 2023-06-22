#pragma once

#include "Hazel/Core/Color.h"
#include "SceneCamera.h"
#include "Hazel/Core/UUID.h"
#include "Hazel/Renderer/Texture.h"
#include "Hazel/Audio/AudioSource.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "Hazel/Math/HMath.h"
#include "Hazel/Renderer/Font.h"

namespace Hazel
{
	//  This component is to be used exclusively for scene's root entity.
	struct Root
	{
		Root() = default;
		Root(const Root& other) = default;
	};

	// Basic Components
#pragma region Basics
	struct IDComponent
	{
		UUID ID;

		IDComponent() = default;
		IDComponent(const UUID uuid) : ID(uuid) {}
		IDComponent(const IDComponent&) = default;
	};

	struct BaseComponent
	{
		std::string Name;
		int Tag = 0;
		int Layer = 0;

		BaseComponent() = default;
		BaseComponent(const BaseComponent&) = default;
	};

	struct FamilyComponent
	{
		UUID ParentID = 0;
		UUID ChildID = 0;
		UUID NextSiblingID = 0;
		UUID PreviousSiblingID = 0;

		FamilyComponent() = default;
		FamilyComponent(const FamilyComponent&) = default;
	};

	// TODO Add static and dynamic so transform for statics do no have to be recalculated all the way down the parent line
	struct TransformComponent 
	{
		glm::vec3 Position{0.0f, 0.0f, 0.0f};
		glm::vec3 Rotation{0.0f, 0.0f, 0.0f};
		glm::vec3 Scale{1.0f, 1.0f, 1.0f};

		const TransformComponent* ParentTransform = nullptr;

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;

		// World Transform
		void SetLocalTransform(const glm::mat4& newTransform)
		{
			glm::vec3 position, rotation, scale;
			if (HMath::DecomposeTransform(newTransform, position, rotation, scale))
			{
				Position = position;
				Rotation = rotation;
				Scale = scale;
			}
		}

		void SetWorldTransform(const glm::mat4& newTransform)
		{
			glm::mat4 newWorldTransform = newTransform;
			if (ParentTransform)
			{
				newWorldTransform = glm::inverse(ParentTransform->GetWorldTransformMatrix()) * newWorldTransform;
			}

			glm::vec3 position, rotation, scale;
			if (HMath::DecomposeTransform(newWorldTransform, position, rotation, scale))
			{
				Position = position;
				Rotation = rotation;
				Scale = scale;
			}
		}

		glm::mat4 GetWorldTransformMatrix() const
		{
			if (ParentTransform)
			{
				return ParentTransform->GetWorldTransformMatrix() * GetLocalTransformMatrix();
			}

			return GetLocalTransformMatrix();
		}

		glm::mat4 GetLocalTransformMatrix() const
		{
			constexpr auto kIdentityMatrix = glm::mat4(1.0f);

			const glm::mat4 localTransformMatrix = glm::translate(kIdentityMatrix, Position)
				* glm::toMat4(glm::quat(Rotation))
				* glm::scale(kIdentityMatrix, Scale);

			return localTransformMatrix;
		}

		TransformComponent GetRelativeTransform() const
		{
			TransformComponent worldTransform;
			glm::vec3 position, rotation, scale;
			if (HMath::DecomposeTransform(GetLocalTransformMatrix(), position, rotation, scale))
			{
				worldTransform.Position = position;
				worldTransform.Rotation = rotation;
				worldTransform.Scale = scale;
			}

			return worldTransform;
		}
	};
#pragma endregion

#pragma region Graphics
	struct SpriteRendererComponent
	{
		Ref<Texture2D> Texture = nullptr;
		glm::vec2 Tiling{1.0f, 1.0f};
		Color Color{Color::White};

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
	};

	struct CircleRendererComponent
	{
		Color Color{Color::White};
		float Thickness = 1.0f;
		float Fade = 0.005f;

		CircleRendererComponent() = default;
		CircleRendererComponent(const CircleRendererComponent&) = default;
	};

	struct TextComponent
	{
		std::string Text;
		Color Color{Color::White};
		float Kerning = 0.0f;
		float LineSpace = 0.0f;
		Ref<Font> FontAsset = Font::GetDefault();
		
		TextComponent() = default;
		TextComponent(const TextComponent&) = default;
	};
#pragma endregion

#pragma region Visual
	struct CameraComponent
	{
		SceneCamera Camera;
		bool IsPrimary = true;
		bool IsFixedAspectRatio = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
	};
#pragma endregion

#pragma region Scripting
	struct ScriptComponent
	{
		std::string ClassName;

		ScriptComponent() = default;
		ScriptComponent(const ScriptComponent&) = default;
	};

	// Forward declaration
	class ScriptableEntity;

	struct NativeScriptComponent
	{
		ScriptableEntity* Instance = nullptr;

		ScriptableEntity* (*InstantiateScript)();
		void (*DestroyScript)(NativeScriptComponent*);

		template<typename T>
		void Bind()
		{
			InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
			DestroyScript = [](NativeScriptComponent* nsc)
			{
				delete nsc->Instance;
				nsc->Instance = nullptr;
			};
		}
	};
#pragma endregion

#pragma region Physics
	struct Rigidbody2DComponent
	{
		enum class BodyType
		{
			Static = 0,
			Dynamic,
			Kinematic
		};

		BodyType Type = BodyType::Static;
		bool IsFixedRotation = false;

		// Storage for runtime
		void* RuntimeBody = nullptr;

		Rigidbody2DComponent() = default;
		Rigidbody2DComponent(const Rigidbody2DComponent&) = default;
	};

	struct BoxCollider2DComponent
	{
		glm::vec2 Offset{0.0f, 0.0f};
		glm::vec2 Size{0.5f, 0.5f};
		float Rotation = 0.0f;

		// TODO move into physics material
		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f;
		float RestitutionThreshold = 0.5f;

		// Storage for runtime
		void* RuntimeFixture = nullptr;

		BoxCollider2DComponent() = default;
		BoxCollider2DComponent(const BoxCollider2DComponent&) = default;
	};

	struct CircleCollider2DComponent
	{
		glm::vec2 Offset{0.0f, 0.0f};
		float Radius = 0.5f;

		// TODO move into physics material
		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f;
		float RestitutionThreshold = 0.5f;

		// Storage for runtime
		void* RuntimeFixture = nullptr;

		CircleCollider2DComponent() = default;
		CircleCollider2DComponent(const CircleCollider2DComponent&) = default;
	};
#pragma endregion

#pragma region Audio
	struct AudioSourceComponent
	{
		Ref<AudioSource> AudioSource;
		bool IsVisibleInGame = false;
		bool IsAutoPlay = false;

		AudioSourceComponent() = default;
		AudioSourceComponent(const AudioSourceComponent&) = default;
	};

	struct AudioListenerComponent
	{
		bool IsVisibleInGame = false;

		AudioListenerComponent() = default;
		AudioListenerComponent(const AudioListenerComponent&) = default;
	};
#pragma endregion

	template<typename... Component>
	struct ComponentGroup {};

	using AllComponents =
	ComponentGroup<TransformComponent,
		SpriteRendererComponent, CircleRendererComponent, TextComponent,
		CameraComponent,
		ScriptComponent, NativeScriptComponent,
		Rigidbody2DComponent, BoxCollider2DComponent, CircleCollider2DComponent,
		AudioSourceComponent, AudioListenerComponent>;
}
