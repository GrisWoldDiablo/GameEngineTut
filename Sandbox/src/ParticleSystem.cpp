#include "ParticleSystem.h"

#include "glm/gtx/compatibility.hpp"

ParticleSystem::ParticleSystem(uint32_t maxParticles)
	:_poolIndex(maxParticles - 1)
{
	_particlePool.resize(maxParticles);
}

void ParticleSystem::OnUpdate(Hazel::Timestep ts)
{
	for (auto& particle : _particlePool)
	{
		if (!particle.Active)
		{
			continue;
		}

		if (particle.LifeRemaining <= 0.0f)
		{
			particle.Active = false;
		}
		particle.LifeRemaining -= ts;
		particle.Position += particle.Velocity * (float)ts;
		particle.Rotation += particle.RotationSpeed * ts;
	}
}

void ParticleSystem::OnRender(Hazel::OrthographicCamera& camera)
{
	Hazel::Renderer2D::BeginScene(camera.GetViewProjectionMatrix());
	for (auto& particle : _particlePool)
	{
		if (!particle.Active)
		{
			continue;
		}

		// Fade particle away
		float life = particle.LifeRemaining / particle.LifeTime;
		auto color = Hazel::Color::Lerp(particle.ColorEnd, particle.ColorBegin, life);

		float size = glm::lerp(particle.SizeEnd, particle.SizeBegin, life);

		Hazel::Renderer2D::DrawRotatedQuad({ particle.Position.x,particle.Position.y, 0.9f }, { size,size }, particle.Rotation, color);

	}
	Hazel::Renderer2D::EndScene();
}

void ParticleSystem::Emit(const ParticleProps& particleProps)
{
	auto startIndex = _poolIndex;
	_poolIndex = --_poolIndex % _particlePool.size();
	Particle& particle = _particlePool[_poolIndex];
	if (particle.Active)
	{
		return;
	}
	particle.Active = true;
	particle.Position = particleProps.Position;
	particle.Rotation = Hazel::Random::Float() * particleProps.RotationSpeedVariation;

	// velocity
	particle.RotationSpeed = particleProps.RotationSpeedVariation * Hazel::Random::Range(-1.0f, 1.0f);
	particle.Velocity = particleProps.Velocity;
	particle.Velocity += particleProps.VelocityVaritation * Hazel::Random::RangeVec2({ -0.5f,0.5f }, { -0.5f,0.5f });

	// color
	particle.ColorBegin = particleProps.ColorBegin;
	particle.ColorEnd = particleProps.ColorEnd;

	particle.LifeTime = particleProps.LifeTime;
	particle.LifeRemaining = particleProps.LifeTime;

	particle.SizeBegin = particleProps.SizeBegin + particleProps.SizeVariation * Hazel::Random::Range(0.0f, 0.5f);
	particle.SizeEnd = particleProps.SizeEnd;
}

void ParticleSystem::SetParticlePoolSize(uint32_t size)
{
	_particlePool.clear();
	_particlePool.resize(size);
}
