#include "Level.h"
#include "glm/gtc/matrix_transform.hpp"

using namespace Hazel;

static bool PointInTriangle(const glm::vec2& p, const glm::vec2& p0, const glm::vec2& p1, const glm::vec2& p2)
{
	float s = p0.y * p2.x - p0.x * p2.y + (p2.y - p0.y) * p.x + (p0.x - p2.x) * p.y;
	float t = p0.x * p1.y - p0.y * p1.x + (p0.y - p1.y) * p.x + (p1.x - p0.x) * p.y;

	if ((s < 0) != (t < 0))
	{
		return false;
	}

	float A = -p1.y * p2.x + p0.y * (p2.x - p1.x) + p0.x * (p1.y - p2.y) + p1.x * p2.y;

	return A < 0 ?
		(s <= 0 && s + t >= A) :
		(s >= 0 && s + t <= A);
}

void Level::Init()
{
	_triangleTexture = Texture2D::Create("assets/textures/Triangle.png");

	_player.LoadAssets();

	_pillars.resize(5);
	for (int i = 0; i < 5; i++)
	{
		CreatePillar(i, i * 10.0f);
	}
}

void Level::OnUpdate(Hazel::Timestep ts)
{
	_player.OnUpdate(ts);

	if (CollisionTest())
	{
		GameOver();
		return;
	}

	_pillarHSV.x += 36.0f * ts;
	if(_pillarHSV.x > 360.0f)
	{
		_pillarHSV.x = 0.0f;
	}

	if (_player.GetPosition().x > _pillarTarget)
	{
		CreatePillar(_pillarIndex, _pillarTarget + 20.0f);
		_pillarIndex = ++_pillarIndex % _pillars.size();
		_pillarTarget += 10.0f;
	}

}

void Level::OnRender()
{
	const auto& playerPos = _player.GetPosition();
	Color color = Color::HSVtoRGB(_pillarHSV);

	// Ceiling
	Renderer2D::DrawQuad({ playerPos.x,34.0f }, { 50.0f,50.0f }, color);
	//  Floor
	Renderer2D::DrawQuad({ playerPos.x,-34.0f }, { 50.0f,50.0f }, color);

	// Pillars
	for (auto& pillar : _pillars)
	{
		Renderer2D::DrawQuad(pillar.TopPosition, pillar.TopScale, 180.0f, _triangleTexture, glm::vec2(1.0f), color);
		Renderer2D::DrawQuad(pillar.BottomPosition, pillar.BottomScale, _triangleTexture, glm::vec2(1.0f), color);
	}

	_player.OnRender();

	//if (CollisionTest())
	//{
	//	Renderer2D::DrawQuad({ playerPos.x,0.0f }, { 1.0f,1.0f }, Color::Red);
	//}

	//for (auto& point : _points)
	//{
	//	Renderer2D::DrawQuad({ point.x,point.y,0.5f }, { 0.4f,0.4f }, Color::Red);
	//}

	//_points.clear();
}

void Level::OnImGuiRender()
{
	_player.OnImGuiRender();
}

void Level::CreatePillar(int index, float offset)
{
	Pillar& pillar = _pillars[index];
	pillar.TopPosition.x = offset;
	pillar.BottomPosition.x = offset;

	float center = Random::Range(-15.0f, 15.0f);
	float gap = Random::Range(1.5f, 2.0f);

	pillar.TopPosition.y = 10.0f - ((10.0f - center) * 0.2f) + gap * 0.5f;
	pillar.BottomPosition.y = -10.0f - ((-10.0f - center) * 0.2f) + gap * 0.5f;
}

bool Level::CollisionTest()
{
	if (abs(_player.GetPosition().y) > 8.5)
	{
		return true;
	}

	glm::vec4 playerVertices[4] =
	{
		{-0.5f, -0.5f, 0.0f, 1.0f},
		{ 0.5f, -0.5f, 0.0f, 1.0f},
		{ 0.5f,  0.5f, 0.0f, 1.0f},
		{-0.5f,  0.5f, 0.0f, 1.0f}
	};

	const auto& playerPos = _player.GetPosition();

	glm::vec4 playerTransformedVerts[4];
	for (int i = 0; i < 4; i++)
	{
		playerTransformedVerts[i] = glm::translate(glm::mat4(1.0f), { playerPos.x, playerPos.y,0.0f })
			* glm::rotate(glm::mat4(1.0f), glm::radians(_player.GetRotation()), { 0.0f,0.0f,1.0f })
			* glm::scale(glm::mat4(1.0f), { 1.0f,1.3f,1.0f })
			* playerVertices[i];

		//_points.emplace_back(playerTransformedVerts[i].x, playerTransformedVerts[i].y);
	}

	glm::vec4 pillarVertices[3] =
	{
		{-0.5f + 0.1f, -0.5f + 0.1f, 0.0f, 1.0f},
		{ 0.5f - 0.1f, -0.5f + 0.1f, 0.0f, 1.0f},
		{ 0.0f + 0.0f,  0.5f - 0.1f, 0.0f, 1.0f},
	};

	for (auto& pillar : _pillars)
	{
		glm::vec2 tri[3];

		// Top Pillars
		for (int i = 0; i < 3; i++)
		{
			tri[i] = glm::translate(glm::mat4(1.0f), { pillar.TopPosition.x, pillar.TopPosition.y, 0.0f })
				* glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), { 0.0f,0.0f,1.0f })
				* glm::scale(glm::mat4(1.0f), { pillar.TopScale.x, pillar.TopScale.y, 1.0f })
				* pillarVertices[i];

			//_points.emplace_back(tri[i].x, tri[i].y);
		}

		// Check here
		for (auto& vert : playerTransformedVerts)
		{
			if (PointInTriangle({ vert.x,vert.y }, tri[0], tri[1], tri[2]))
			{
 				return true;
			}
		}

		// Bottom Pillars
		for (int i = 0; i < 3; i++)
		{
			tri[i] = glm::translate(glm::mat4(1.0f), { pillar.BottomPosition.x, pillar.BottomPosition.y, 0.0f })
				* glm::scale(glm::mat4(1.0f), { pillar.BottomScale.x, pillar.BottomScale.y, 1.0f })
				* pillarVertices[i];

			//_points.emplace_back(tri[i].x, tri[i].y);
		}

		// Check here
		for (auto& vert : playerTransformedVerts)
		{
			if (PointInTriangle({ vert.x,vert.y }, tri[0], tri[1], tri[2]))
			{
				return true;
			}
		}
	}

	return false;
}

void Level::GameOver()
{
	_player.Reset();

	_pillarTarget = 30.0f;
	_pillarIndex = 0;
	for (int i = 0; i < 5; i++)
	{
		CreatePillar(i, i * 10.0f);
	}
}
