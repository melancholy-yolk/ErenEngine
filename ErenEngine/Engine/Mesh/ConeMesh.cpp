#include "ConeMesh.h"
#include "Core/MeshType.h"

void CConeMesh::Init()
{
	Super::Init();

	WorldMatrix = XMFLOAT4X4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
}

void CConeMesh::BuildMesh(const FMeshRenderingData* InRenderingData)
{
	Super::BuildMesh(InRenderingData);

}

void CConeMesh::Draw(float DeltaTime)
{
	Super::Draw(DeltaTime);

}

/// <summary>
/// 球面坐标转三维空间坐标
/// </summary>
/// <param name="InRadius">球半径</param>
/// <param name="InAxisSubdivision">轴向细分</param>
/// <param name="InHeightSubdivision">高度向细分</param>
/// <returns></returns>
CConeMesh* CConeMesh::CreateMesh(
	float InRadius,
	float InHeight,
	uint32_t InAxisSubdivision,
	uint32_t InHeightSubdivision
)
{
	FMeshRenderingData MeshData;
	
	float RadiusInterval = InRadius / InHeightSubdivision;
	float HeightInterval = InHeight / InHeightSubdivision;
	float BetaValue = XM_2PI / InAxisSubdivision;

	// 圆锥最顶上的点
	MeshData.VertexData.push_back(
		FVertex(
			XMFLOAT3(0, 0.5f * InHeight, 0),
			XMFLOAT4(Colors::White)
		)
	);

	for (uint32_t i = 1; i <= InHeightSubdivision; i++)
	{
		float Y = 0.5f * InHeight - HeightInterval * i;
		float Radius = RadiusInterval * i;
		for (uint32_t j = 0; j < InAxisSubdivision; j++)
		{
			float Beta = (j + 1) * BetaValue;
			MeshData.VertexData.push_back(
				FVertex(
					XMFLOAT3(
						Radius * cosf(Beta),
						Y,
						Radius * sinf(Beta)
					),
					XMFLOAT4(Colors::White)
				)
			);
		}
	}

	// 圆锥底面圆心顶点
	MeshData.VertexData.push_back(
		FVertex(
			XMFLOAT3(0, -0.5f * InHeight, 0),
			XMFLOAT4(Colors::White)
		)
	);

	// 绘制圆锥最上面一条
	uint32_t BaseIndex = 1;
	for (uint32_t i = 0; i < InAxisSubdivision; i++)
	{
		MeshData.IndexData.push_back(0);
		MeshData.IndexData.push_back(BaseIndex + (i + 1) % InAxisSubdivision);
		MeshData.IndexData.push_back(BaseIndex + i);
	}

	// 绘制圆锥除最上面一条外，其余条
	uint32_t N = InAxisSubdivision;// 轴向细分数量
	BaseIndex = 1;
	for (uint32_t i = 0; i < InHeightSubdivision - 1; i++)
	{
		for (uint32_t j = 0; j < InAxisSubdivision; j++)
		{
			// 绘制一个四边形 顺时针为正面
			MeshData.IndexData.push_back(BaseIndex + i * N + j);// 左上
			MeshData.IndexData.push_back(BaseIndex + (i + 1) * N + (j + 1) % N);// 右下
			MeshData.IndexData.push_back(BaseIndex + (i + 1) * N + j);// 左下

			MeshData.IndexData.push_back(BaseIndex + i * N + j);// 左上
			MeshData.IndexData.push_back(BaseIndex + i * N + (j + 1) % N);// 右上
			MeshData.IndexData.push_back(BaseIndex + (i + 1) * N + (j + 1) % N);// 右下
			
		}
	}

	// 绘制圆锥底面
	uint32_t LastIndex = MeshData.VertexData.size() - 1 - InAxisSubdivision;
	for (uint32_t i = 0; i < InAxisSubdivision; i++)
	{
		MeshData.IndexData.push_back(LastIndex + InAxisSubdivision);
		MeshData.IndexData.push_back(LastIndex + i);
		MeshData.IndexData.push_back(LastIndex + (i + 1) % InAxisSubdivision);
	}

	CConeMesh* ConeMesh = new CConeMesh;
	ConeMesh->BuildMesh(&MeshData);
	ConeMesh->Init();

	return ConeMesh;
}
