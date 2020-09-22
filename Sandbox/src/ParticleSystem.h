#pragma once

#include <Hazel.h>

struct ParticleProps
{
	glm::vec2 Position;
	glm::vec2 Velocity;
	glm::vec2 VelocityVaritation;
	Hazel::Color ColorBegin;
	Hazel::Color ColorEnd;
	float SizeBegin;
	float SizeEnd;
	float SizeVariation;
	float RotationSpeedVariation;
	float LifeTime = 1.0f;
};

class ParticleSystem
{
public:
	ParticleSystem(uint32_t maxParticles = 100000);

	void OnUpdate(Hazel::Timestep ts);
	void OnRender(Hazel::OrthographicCamera& camera);

	void Emit(const ParticleProps& particleProps);
	void SetParticlePoolSize(uint32_t size);

private:
	struct Particle
	{
		glm::vec2 Position;
		glm::vec2 Velocity;
		glm::vec2 VelocityVaritation;
		Hazel::Color ColorBegin;
		Hazel::Color ColorEnd;
		float Rotation = 0.0f;
		float RotationSpeed = 1.0f;
		float SizeBegin;
		float SizeEnd;

		float LifeTime = 1.0f;
		float LifeRemaining = 0.0f;

		bool Active = false;
	};
	std::vector<Particle> _particlePool;
	uint32_t _poolIndex;
};

