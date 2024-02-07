#include "CylinderMesh.h"
#include "Core/MeshType.h"

void CCylinderMesh::Init()
{
	Super::Init();

	WorldMatrix = XMFLOAT4X4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
}

void CCylinderMesh::BuildMesh(const FMeshRenderingData* InRenderingData)
{
	Super::BuildMesh(InRenderingData);

}

void CCylinderMesh::Draw(float DeltaTime)
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
CCylinderMesh* CCylinderMesh::CreateMesh(
	float InTopRadius,
	float InBottomRadius,
	float InHeight,
	uint32_t InAxisSubdivision,
	uint32_t InHeightSubdivision
)
{
	FMeshRenderingData MeshData;
	
	float RadiusInterval = (InBottomRadius - InTopRadius) / InHeightSubdivision;
	float HeightInterval = InHeight / InHeightSubdivision;
	float BetaValue = XM_2PI / InAxisSubdivision;

	for (uint32_t i = 0; i <= InHeightSubdivision; i++)
	{
		float Y = 0.5f * InHeight - HeightInterval * i;
		float Radius = InTopRadius + RadiusInterval * i;
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

	uint32_t N = InAxisSubdivision;// 轴向细分数量

	for (uint32_t i = 0; i < InHeightSubdivision; i++)
	{
		for (uint32_t j = 0; j < InAxisSubdivision; j++)
		{
			// 绘制一个四边形 顺时针为正面
			MeshData.IndexData.push_back(i * N + j);// 左上
			MeshData.IndexData.push_back((i + 1) * N + (j + 1) % N);// 右下
			MeshData.IndexData.push_back((i + 1) * N + j);// 左下

			MeshData.IndexData.push_back(i * N + j);// 左上
			MeshData.IndexData.push_back(i * N + (j + 1) % N);// 右上
			MeshData.IndexData.push_back((i + 1) * N + (j + 1) % N);// 右下
			
		}
	}

	// 绘制顶部
	{
		for (uint32_t i = 0; i < InAxisSubdivision; i++)
		{
			float Beta = (i + 1) * BetaValue;
			MeshData.VertexData.push_back(
				FVertex(
					XMFLOAT3(
						InTopRadius * cosf(Beta),
						0.5f * InHeight,
						InTopRadius * sinf(Beta)
					),
					XMFLOAT4(Colors::White)
				)
			);
		}
		//添加顶部圆心顶点
		MeshData.VertexData.push_back(
			FVertex(
				XMFLOAT3(0, 0.5f * InHeight, 0),
				XMFLOAT4(Colors::White)
			)
		);

		uint32_t BaseIndex = (InHeightSubdivision + 1) * InAxisSubdivision;

		for (uint32_t i = 0; i < InAxisSubdivision; i++)
		{
			MeshData.IndexData.push_back(BaseIndex + InAxisSubdivision);
			MeshData.IndexData.push_back(BaseIndex + i + 1);
			MeshData.IndexData.push_back(BaseIndex + i);
		}
	}

	// 绘制底部
	{
		for (uint32_t i = 0; i < InAxisSubdivision; i++)
		{
			float Beta = (i + 1) * BetaValue;
			MeshData.VertexData.push_back(
				FVertex(
					XMFLOAT3(
						InBottomRadius * cosf(Beta),
						-0.5f * InHeight,
						InBottomRadius * sinf(Beta)
					),
					XMFLOAT4(Colors::White)
				)
			);
		}
		//添加底部圆心顶点
		MeshData.VertexData.push_back( 
			FVertex(
				XMFLOAT3(0, -0.5f * InHeight, 0),
				XMFLOAT4(Colors::White)
			)
		);

		uint32_t BaseIndex = (InHeightSubdivision + 1) * InAxisSubdivision + InAxisSubdivision + 1;

		for (uint32_t i = 0; i < InAxisSubdivision; i++)
		{
			MeshData.IndexData.push_back(BaseIndex + InAxisSubdivision);
			MeshData.IndexData.push_back(BaseIndex + i);
			MeshData.IndexData.push_back(BaseIndex + i + 1);
			
		}
	}

	CCylinderMesh* CylinderMesh = new CCylinderMesh;
	CylinderMesh->BuildMesh(&MeshData);
	CylinderMesh->Init();

	return CylinderMesh;
}
