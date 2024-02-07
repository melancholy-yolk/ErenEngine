#include "MeshType.h"

FVertex::FVertex(const XMFLOAT3& InPos, const XMFLOAT4& InColor)
	: Position(InPos)
	, Color(InColor)
	, Normal(InPos)
{

}

FVertex::FVertex(const XMFLOAT3& InPos, const XMFLOAT4& InColor, const XMFLOAT3& InNormal)
	: Position(InPos)
	, Color(InColor)
	, Normal(InNormal)
{

}