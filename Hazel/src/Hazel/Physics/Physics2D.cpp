#include "hzpch.h"
#include "Physics2D.h"

#include "box2d/b2_body.h"

b2BodyType Hazel::Utils::TypeToBox2DBody(const Rigidbody2DComponent::BodyType bodyType)
{
	switch (bodyType)
	{
	case Rigidbody2DComponent::BodyType::Static: return b2_staticBody;
	case Rigidbody2DComponent::BodyType::Dynamic: return b2_dynamicBody;
	case Rigidbody2DComponent::BodyType::Kinematic: return b2_kinematicBody;
	}

	HZ_CORE_ASSERT(false, "Unknown body Type!");
	return b2_staticBody;
}

Hazel::Rigidbody2DComponent::BodyType Hazel::Utils::Box2DBodyToType(const b2BodyType bodyType)
{
	switch (bodyType)
	{
	case b2_staticBody: return Rigidbody2DComponent::BodyType::Static;
	case b2_dynamicBody: return Rigidbody2DComponent::BodyType::Dynamic;
	case b2_kinematicBody: return Rigidbody2DComponent::BodyType::Kinematic;
	}

	HZ_CORE_ASSERT(false, "Unknown body Type!");
	return Rigidbody2DComponent::BodyType::Static;
}
