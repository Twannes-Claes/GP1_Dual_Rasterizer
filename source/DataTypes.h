#pragma once
#include "Math.h"
#include "vector"

namespace dae
{
	struct Vertex
	{
		Vector3 position{};
		Vector3 normal{};
		Vector3 tangent{};
		Vector2 uv{};
		ColorRGB color{colors::White};
		Vector3 viewDirection{};
	};

	struct Vertex_Out
	{
		Vector4 position{};
		Vector3 normal{};
		Vector3 tangent{};
		Vector2 uv{};
		ColorRGB color{ colors::White };
		Vector3 viewDirection{};
	};

	struct AABB
	{
		Vector2 minAABB{};
		Vector2 maxAABB{};
	};

	enum class PrimitiveTopology
	{
		TriangleList,
		TriangleStrip
	};

	enum class CullMode
	{
		back,
		front,
		none
	};

}