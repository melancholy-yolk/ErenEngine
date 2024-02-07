#pragma once
#include "Core/Mesh.h"

class CCustomMesh : public CMesh
{
	typedef CMesh Super;
public:
	virtual void Init();
	virtual void BuildMesh(const FMeshRenderingData* InRenderingData);
	virtual void Draw(float DeltaTime);

	static CCustomMesh* CreateMesh(string InPath);

	static bool LoadObjFromBuffer(char* InBuffer, uint32_t InBufferSize, FMeshRenderingData& MeshData);
};