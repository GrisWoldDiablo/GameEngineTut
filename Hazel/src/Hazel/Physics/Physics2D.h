#pragma once

#include "Hazel/Scene/Components.h"

enum b2BodyType : int;

namespace Hazel::Utils
{
	b2BodyType TypeToBox2DBody(const Rigidbody2DComponent::BodyType bodyType);

	Rigidbody2DComponent::BodyType Box2DBodyToType(const b2BodyType bodyType);
}
