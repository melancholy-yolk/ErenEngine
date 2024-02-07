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
/// ��������ת��ά�ռ�����
/// </summary>
/// <param name="InRadius">��뾶</param>
/// <param name="InAxisSubdivision">����ϸ��</param>
/// <param name="InHeightSubdivision">�߶���ϸ��</param>
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

	// Բ׶��ϵĵ�
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

	// Բ׶����Բ�Ķ���
	MeshData.VertexData.push_back(
		FVertex(
			XMFLOAT3(0, -0.5f * InHeight, 0),
			XMFLOAT4(Colors::White)
		)
	);

	// ����Բ׶������һ��
	uint32_t BaseIndex = 1;
	for (uint32_t i = 0; i < InAxisSubdivision; i++)
	{
		MeshData.IndexData.push_back(0);
		MeshData.IndexData.push_back(BaseIndex + (i + 1) % InAxisSubdivision);
		MeshData.IndexData.push_back(BaseIndex + i);
	}

	// ����Բ׶��������һ���⣬������
	uint32_t N = InAxisSubdivision;// ����ϸ������
	BaseIndex = 1;
	for (uint32_t i = 0; i < InHeightSubdivision - 1; i++)
	{
		for (uint32_t j = 0; j < InAxisSubdivision; j++)
		{
			// ����һ���ı��� ˳ʱ��Ϊ����
			MeshData.IndexData.push_back(BaseIndex + i * N + j);// ����
			MeshData.IndexData.push_back(BaseIndex + (i + 1) * N + (j + 1) % N);// ����
			MeshData.IndexData.push_back(BaseIndex + (i + 1) * N + j);// ����

			MeshData.IndexData.push_back(BaseIndex + i * N + j);// ����
			MeshData.IndexData.push_back(BaseIndex + i * N + (j + 1) % N);// ����
			MeshData.IndexData.push_back(BaseIndex + (i + 1) * N + (j + 1) % N);// ����
			
		}
	}

	// ����Բ׶����
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
