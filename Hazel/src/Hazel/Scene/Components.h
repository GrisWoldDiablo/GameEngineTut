#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Hazel/Core/Color.h"
#include "SceneCamera.h"
#include "ScriptableEntity.h"
#include "Hazel/Renderer/Texture.h"

namespace Hazel
{
	struct TagComponent
	{
		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(std::string tag)
			:Tag(std::move(tag))
		{}
	};

	struct TransformComponent
	{
		glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::vec3& position)
			:Position(position)
		{}

		glm::mat4 GetTransform() const
		{
			auto identityMatrix = glm::mat4(1.0f);
			auto rotation = glm::rotate(identityMatrix, Rotation.x, { 1, 0, 0 })
				* glm::rotate(identityMatrix, Rotation.y, { 0, 1, 0 })
				* glm::rotate(identityMatrix, Rotation.z, { 0, 0, 1 });

			return glm::translate(identityMatrix, Position)
				* rotation
				* glm::scale(identityMatrix, Scale);
		}
	};

	struct SpriteRendererComponent
	{
		Ref<Texture2D> Texture = nullptr;
		glm::vec2 Tiling{ 1.0f, 1.0f };
		Color Color{ Color::White };

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(const Hazel::Color& color)
			:Color(color)
		{}

		SpriteRendererComponent(const Ref<Texture2D> texture)
			:Texture(texture)
		{}
	};

	struct CameraComponent
	{
		SceneCamera Camera;
		bool IsPrimary = true; // TODO Think about moving to scene.
		bool IsFixedAspectRatio = false;

		CameraComponent() = default;
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
