#include "hzpch.h"
#include "Components.h"
#include "box2d/b2_body.h"

namespace Hazel
{
	uint8_t Rigidbody2DComponent::TypeToBox2DBody(Rigidbody2DComponent::BodyType bodyType)
	{
		switch (bodyType)
		{
		case Hazel::Rigidbody2DComponent::BodyType::Static: return b2_staticBody;
		case Hazel::Rigidbody2DComponent::BodyType::Dynamic: return b2_dynamicBody;
		case Hazel::Rigidbody2DComponent::BodyType::Kinematic: return b2_kinematicBody;
		}

		HZ_CORE_ASSERT(false, "Unknown body Type!");
		return b2_staticBody;
	}

	Hazel::Rigidbody2DComponent::BodyType Rigidbody2DComponent::Box2DBodyToType(uint8_t bodyType)
	{
		switch (bodyType)
		{
		case b2_staticBody: return Hazel::Rigidbody2DComponent::BodyType::Static;
		case b2_dynamicBody: return Hazel::Rigidbody2DComponent::BodyType::Dynamic;
		case b2_kinematicBody: return Hazel::Rigidbody2DComponent::BodyType::Kinematic;
		}

		HZ_CORE_ASSERT(false, "Unknown body Type!");
		return Hazel::Rigidbody2DComponent::BodyType::Static;
	}
}
