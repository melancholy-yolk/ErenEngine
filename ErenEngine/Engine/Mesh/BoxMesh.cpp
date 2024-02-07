#include "BoxMesh.h"
#include "Core/MeshType.h"

void CBoxMesh::Init()
{
	Super::Init();

	WorldMatrix = XMFLOAT4X4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
}

void CBoxMesh::BuildMesh(const FMeshRenderingData* InRenderingData)
{
	Super::BuildMesh(InRenderingData);

}

void CBoxMesh::Draw(float DeltaTime)
{
	Super::Draw(DeltaTime);

}

CBoxMesh* CBoxMesh::CreateMesh(float InHeight, float InWidth, float InDepth)
{
	FMeshRenderingData MeshData;

	float CHeight = 0.5f * InHeight;
	float CWidth = 0.5f * InWidth;
	float CDepth = 0.5f * InDepth;

	//构建VertexBuffer
	MeshData.VertexData.push_back(FVertex(XMFLOAT3(-CWidth, -CHeight, -CDepth), XMFLOAT4(Colors::White)));
	MeshData.VertexData.push_back(FVertex(XMFLOAT3(-CWidth, CHeight, -CDepth), XMFLOAT4(Colors::AliceBlue)));
	MeshData.VertexData.push_back(FVertex(XMFLOAT3(CWidth, CHeight, -CDepth), XMFLOAT4(Colors::Aqua)));
	MeshData.VertexData.push_back(FVertex(XMFLOAT3(CWidth, -CHeight, -CDepth), XMFLOAT4(Colors::Aquamarine)));
	MeshData.VertexData.push_back(FVertex(XMFLOAT3(-CWidth, -CHeight, CDepth), XMFLOAT4(Colors::Bisque)));
	MeshData.VertexData.push_back(FVertex(XMFLOAT3(-CWidth, CHeight, CDepth), XMFLOAT4(Colors::Blue)));
	MeshData.VertexData.push_back(FVertex(XMFLOAT3(CWidth, CHeight, CDepth), XMFLOAT4(Colors::Chocolate)));
	MeshData.VertexData.push_back(FVertex(XMFLOAT3(CWidth, -CHeight, CDepth), XMFLOAT4(Colors::Chocolate)));

	//构建IndexBuffer
	// Front
	MeshData.IndexData.push_back(0); MeshData.IndexData.push_back(1); MeshData.IndexData.push_back(2);
	MeshData.IndexData.push_back(0); MeshData.IndexData.push_back(2); MeshData.IndexData.push_back(3);

	// Back
	MeshData.IndexData.push_back(4); MeshData.IndexData.push_back(6); MeshData.IndexData.push_back(5);
	MeshData.IndexData.push_back(4); MeshData.IndexData.push_back(7); MeshData.IndexData.push_back(6);

	// Left
	MeshData.IndexData.push_back(4); MeshData.IndexData.push_back(5); MeshData.IndexData.push_back(1);
	MeshData.IndexData.push_back(4); MeshData.IndexData.push_back(1); MeshData.IndexData.push_back(0);

	// Right
	MeshData.IndexData.push_back(3); MeshData.IndexData.push_back(2); MeshData.IndexData.push_back(6);
	MeshData.IndexData.push_back(3); MeshData.IndexData.push_back(6); MeshData.IndexData.push_back(7);

	// Bottom
	MeshData.IndexData.push_back(1); MeshData.IndexData.push_back(5); MeshData.IndexData.push_back(6);
	MeshData.IndexData.push_back(1); MeshData.IndexData.push_back(6); MeshData.IndexData.push_back(2);

	// Up
	MeshData.IndexData.push_back(4); MeshData.IndexData.push_back(0); MeshData.IndexData.push_back(3);
	MeshData.IndexData.push_back(4); MeshData.IndexData.push_back(3); MeshData.IndexData.push_back(7);

	CBoxMesh* BoxMesh = new CBoxMesh;
	BoxMesh->BuildMesh(&MeshData);
	BoxMesh->Init();

	return BoxMesh;
}
