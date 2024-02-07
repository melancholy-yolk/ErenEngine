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
/// ��������ת��ά�ռ�����
/// </summary>
/// <param name="InRadius">��뾶</param>
/// <param name="InAxisSubdivision">����ϸ��</param>
/// <param name="InHeightSubdivision">�߶���ϸ��</param>
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

	uint32_t N = InAxisSubdivision;// ����ϸ������

	for (uint32_t i = 0; i < InHeightSubdivision; i++)
	{
		for (uint32_t j = 0; j < InAxisSubdivision; j++)
		{
			// ����һ���ı��� ˳ʱ��Ϊ����
			MeshData.IndexData.push_back(i * N + j);// ����
			MeshData.IndexData.push_back((i + 1) * N + (j + 1) % N);// ����
			MeshData.IndexData.push_back((i + 1) * N + j);// ����

			MeshData.IndexData.push_back(i * N + j);// ����
			MeshData.IndexData.push_back(i * N + (j + 1) % N);// ����
			MeshData.IndexData.push_back((i + 1) * N + (j + 1) % N);// ����
			
		}
	}

	// ���ƶ���
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
		//��Ӷ���Բ�Ķ���
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

	// ���Ƶײ�
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
		//��ӵײ�Բ�Ķ���
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
