#include "CustomMesh.h"
#include "Core/MeshType.h"

void CCustomMesh::Init()
{
	Super::Init();

	// 行向量 乘以 列矩阵
	WorldMatrix = XMFLOAT4X4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
}

void CCustomMesh::BuildMesh(const FMeshRenderingData* InRenderingData)
{
	Super::BuildMesh(InRenderingData);

}

void CCustomMesh::Draw(float DeltaTime)
{
	Super::Draw(DeltaTime);

}

CCustomMesh* CCustomMesh::CreateMesh(string InPath)
{
	FMeshRenderingData MeshData;

	{
		uint32_t FileSize = get_file_size_by_filename(InPath.c_str());

		char* Buffer = new char[FileSize + 1];
		memset(Buffer, 0, FileSize + 1);

		get_file_buf(InPath.c_str(), Buffer);
		if (!LoadObjFromBuffer(Buffer, FileSize, MeshData))
		{

		}
		delete Buffer;
	}

	CCustomMesh* CustomMesh = new CCustomMesh;
	CustomMesh->BuildMesh(&MeshData);
	CustomMesh->Init();

	return CustomMesh;
}

bool CCustomMesh::LoadObjFromBuffer(char* InBuffer, uint32_t InBufferSize, FMeshRenderingData& MeshData)
{
	if (InBufferSize > 0)
	{
		stringstream BufferStream(InBuffer);
		char TmpLine[256] = {0};
		string LineHead;

		//逐行提取字符
		for (;!BufferStream.eof();)
		{
			memset(TmpLine, 0, 256);
			BufferStream.getline(TmpLine, 256);
			if (strlen(TmpLine) > 0)
			{
				// 方便进行字符串操作
				stringstream LineStream(TmpLine);

				if (TmpLine[0] == 'v')
				{
					LineStream >> LineHead;

					if (TmpLine[1] == 'n')
					{

					}
					else if (TmpLine[1] == 't')
					{

					}
					else
					{
						MeshData.VertexData.push_back(FVertex(XMFLOAT3(), XMFLOAT4(Colors::White)));
						int TopIndex = MeshData.VertexData.size() - 1;
						XMFLOAT3& Float3InPos = MeshData.VertexData[TopIndex].Position;

						LineStream >> Float3InPos.x;
						LineStream >> Float3InPos.y;
						LineStream >> Float3InPos.z;
					}
				}
				else if (TmpLine[0] == 'f')
				{
					LineStream >> LineHead;

					char SaveLineString[256] = { 0 };
					char TempBuffer[256] = { 0 };

					for (uint32_t i = 0; i < 3; i++)
					{
						memset(SaveLineString, 0, 256);
						//解析对应单个顶点的索引信息（顶点索引/UV索引/法线索引）
						LineStream >> SaveLineString;
						uint32_t Strlen = strlen(SaveLineString);

						//解析顶点索引
						int FirstDelimitPos = find_string(SaveLineString, "/", 0);
						memset(TempBuffer, 0, 256);
						char *VertexIndexStr = string_mid(SaveLineString, TempBuffer, 0, FirstDelimitPos);
						int VertexIndex = atoi(VertexIndexStr);
						MeshData.IndexData.push_back(VertexIndex);

						//解析纹理索引
						int SecondDelimitPos = find_string(SaveLineString, "/", FirstDelimitPos + 1);
						memset(TempBuffer, 0, 256);
						char *TextureIndexStr = string_mid(SaveLineString, TempBuffer, FirstDelimitPos + 1, SecondDelimitPos - (FirstDelimitPos + 1));
						int TextureIndex = atoi(TextureIndexStr);

						//解析纹理索引
						memset(TempBuffer, 0, 256);
						char *NormalIndexStr = string_mid(SaveLineString, TempBuffer, SecondDelimitPos + 1, Strlen - (SecondDelimitPos + 1));
						int NormalIndex = atoi(NormalIndexStr);
					}

					
				}
			}
		}
	}

	return false;
}