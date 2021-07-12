#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "Hazel/Core/Color.h"
#include "SceneCamera.h"
#include "ScriptableEntity.h"
#include "Hazel/Renderer/Texture.h"

namespace Hazel
{
	struct Component
	{
	public:
		Component(Entity entity)
			:_entity(entity)
		{}
		virtual ~Component() = default;

	public:
		Entity GetEntity() { return _entity; }

	private:
		Entity _entity;
	};

	struct BaseComponent : Component
	{
		std::string Name;
		int Tag;
		int Layer;

		BaseComponent(Entity entity) : Component(entity) {};
		BaseComponent(const BaseComponent&) = default;

		~BaseComponent() = default;
	};

	struct TransformComponent : Component
	{
		glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };

		TransformComponent(Entity entity) : Component(entity) {};
		TransformComponent(const TransformComponent&) = default;

		glm::mat4 GetTransformMatrix() const
		{
			const auto kIdentityMatrix = glm::mat4(1.0f);
			glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));
			// This give different outcome.
			/*auto rotation = glm::rotate(identityMatrix, Rotation.x, { 1, 0, 0 })
				* glm::rotate(identityMatrix, Rotation.y, { 0, 1, 0 })
				* glm::rotate(identityMatrix, Rotation.z, { 0, 0, 1 });*/

			return glm::translate(kIdentityMatrix, Position)
				* rotation
				* glm::scale(kIdentityMatrix, Scale);
		}
	};

	struct SpriteRendererComponent : Component
	{
		Ref<Texture2D> Texture = nullptr;
		glm::vec2 Tiling{ 1.0f, 1.0f };
		Color Color{ Color::White };

		SpriteRendererComponent(Entity entity) : Component(entity) {};
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
	};

	struct CameraComponent : Component
	{
		SceneCamera Camera;
		bool IsPrimary = true;
		bool IsFixedAspectRatio = false;

		CameraComponent(Entity entity) : Component(entity) {};
		CameraComponent(const CameraComponent&) = default;
	};

	struct NativeScriptComponent
	{
		ScriptableEntity* Instance = nullptr;

		ScriptableEntity* (*InstantiateScript)();
		void (*DestroyScript)(NativeScriptComponent*);

		template<typename T>
		void Bind()
		{
			InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
			DestroyScript = [](NativeScriptComponent* nsc) { delete  nsc->Instance; nsc->Instance = nullptr; };
		}
	};
}
