#include "PlaneMesh.h"
#include "Core/MeshType.h"

void CPlaneMesh::Init()
{
	Super::Init();

	WorldMatrix = XMFLOAT4X4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
}

void CPlaneMesh::BuildMesh(const FMeshRenderingData* InRenderingData)
{
	Super::BuildMesh(InRenderingData);

}

void CPlaneMesh::Draw(float DeltaTime)
{
	Super::Draw(DeltaTime);

}

CPlaneMesh* CPlaneMesh::CreateMesh(float InHeight, float InWidth, uint32_t InHeightSubdivision, uint32_t InWidthSubdivision)
{
	FMeshRenderingData MeshData;

	auto SubdivideValue = [](float InValue, uint32_t InSubdivideValue)->float
	{
		if (InSubdivideValue <= 1)
		{
			return InValue;
		}
		return InValue / ((float)InSubdivideValue);
	};

	float CHeight = 0.5f * InHeight;
	float CWidth = 0.5f * InWidth;

	float HeightSubdivideValue = SubdivideValue(InHeight, InHeightSubdivision);
	float WidthSubdivideValue = SubdivideValue(InWidth, InWidthSubdivision);

	for (uint32_t i = 0; i <= InHeightSubdivision; i++)
	{
		for (uint32_t j = 0; j <= InWidthSubdivision; j++)
		{
			MeshData.VertexData.push_back(FVertex(
				XMFLOAT3(-CWidth + j * WidthSubdivideValue, 0, CHeight - i * HeightSubdivideValue),
				XMFLOAT4(Colors::White)
			));
		}
	}

	uint32_t N = InWidthSubdivision + 1;// 轴向细分数量

	for (uint32_t i = 0; i < InHeightSubdivision; i++)
	{
		for (uint32_t j = 0; j < InWidthSubdivision; j++)
		{
			// 绘制一个四边形 顺时针为正面
			MeshData.IndexData.push_back(i * N + j);// 左上
			MeshData.IndexData.push_back((i + 1) * N + (j + 1));// 右下
			MeshData.IndexData.push_back((i + 1) * N + j);// 左下

			MeshData.IndexData.push_back(i * N + j);// 左上
			MeshData.IndexData.push_back(i * N + (j + 1));// 右上
			MeshData.IndexData.push_back((i + 1) * N + (j + 1));// 右下

		}
	}

	CPlaneMesh* PlaneMesh = new CPlaneMesh;
	PlaneMesh->BuildMesh(&MeshData);
	PlaneMesh->Init();

	return PlaneMesh;
}
