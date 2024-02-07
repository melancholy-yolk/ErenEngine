#include "SphereMesh.h"
#include "Core/MeshType.h"

void CSphereMesh::Init()
{
	Super::Init();

	WorldMatrix = XMFLOAT4X4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
}

void CSphereMesh::BuildMesh(const FMeshRenderingData* InRenderingData)
{
	Super::BuildMesh(InRenderingData);

}

void CSphereMesh::Draw(float DeltaTime)
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
CSphereMesh* CSphereMesh::CreateMesh(float InRadius, uint32_t InAxisSubdivision, uint32_t InHeightSubdivision)
{
	FMeshRenderingData MeshData;
	
	float BetaValue = XM_2PI / InAxisSubdivision;
	float ThetaValue = XM_PI / InHeightSubdivision;

	XMFLOAT3 xNormal(0, InRadius, 0);
	XMVECTOR Normal = XMLoadFloat3(&xNormal);
	XMStoreFloat3(&xNormal, XMVector3Normalize(Normal));

	//��������
	MeshData.VertexData.push_back(FVertex(
		XMFLOAT3(0, InRadius, 0),
		XMFLOAT4(Colors::White)
	));

	// �ӱ������ϼ� �������� һȦһȦ�Ļ���
	for (uint32_t i = 0; i < InHeightSubdivision; i++)
	{
		float Theta = (i + 1) * ThetaValue;

		XMFLOAT4 Color = i == InHeightSubdivision - 1 ? XMFLOAT4(Colors::Red) : XMFLOAT4(Colors::White);
		
		for (uint32_t j = 0; j < InAxisSubdivision; j++)
		{
			float Beta = (j + 1) * BetaValue;

			MeshData.VertexData.push_back(FVertex(
				XMFLOAT3(
					InRadius * sinf(Theta) * cosf(Beta),
					InRadius * cosf(Theta),
					InRadius * sinf(Theta) * sinf(Beta)),
				Color
			));

			int TopIndex = MeshData.VertexData.size() - 1;

			XMVECTOR Pos = XMLoadFloat3(&MeshData.VertexData[TopIndex].Position);
			XMStoreFloat3(&MeshData.VertexData[TopIndex].Normal, XMVector3Normalize(Pos));
		}
	}

	// ���Ʊ��� �����㶥������Ϊ0
	for (uint32_t i = 0; i < InAxisSubdivision; ++i)
	{
		uint32_t Index = i + 1;

		MeshData.IndexData.push_back(0);
		if (Index == InAxisSubdivision)
		{
			MeshData.IndexData.push_back(1);
		}
		else
		{
			MeshData.IndexData.push_back(Index + 1);
		}
		MeshData.IndexData.push_back(Index);
	}

	uint32_t BaseIndex = 1;
	uint32_t VertexCircleNum = InAxisSubdivision;// ����ϸ������

	for (uint32_t i = 0; i < InHeightSubdivision - 1; i++)
	{
		for (uint32_t j = 0; j < InAxisSubdivision; j++)
		{
			bool IsTail = (j + 1 == InAxisSubdivision);//�Ƿ�ͷβ��ӵĶ���
			MeshData.IndexData.push_back(BaseIndex + i * VertexCircleNum + j);
			MeshData.IndexData.push_back(BaseIndex + i * VertexCircleNum + (j + 1) % VertexCircleNum);
			MeshData.IndexData.push_back(BaseIndex + (i + 1) * VertexCircleNum + j);

			MeshData.IndexData.push_back(BaseIndex + (i + 1) * VertexCircleNum + j);
			MeshData.IndexData.push_back(BaseIndex + i * VertexCircleNum + (j + 1) % VertexCircleNum);
			MeshData.IndexData.push_back(BaseIndex + (i + 1) * VertexCircleNum + (j + 1) % VertexCircleNum);
		}
	}

	// �����ϼ�
	//uint32_t SouthBaseIndex = MeshData.VertexData.size() - 1;// ���һ��������
	//BaseIndex = SouthBaseIndex - VertexCircleNum;
	//for (uint32_t i = 0; i < InAxisSubdivision; ++i)
	//{
	//	MeshData.IndexData.push_back(SouthBaseIndex);
	//	MeshData.IndexData.push_back(BaseIndex + i);
	//	MeshData.IndexData.push_back(BaseIndex + i + 1);
	//}

	CSphereMesh* SphereMesh = new CSphereMesh;
	SphereMesh->BuildMesh(&MeshData);
	SphereMesh->Init();

	return SphereMesh;
}
