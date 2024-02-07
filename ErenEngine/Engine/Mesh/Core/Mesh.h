#pragma once
#include "../../Rendering/Core/Rendering.h"
#include "MeshType.h"
#include "../../Shader/Core/Shader.h"

// ����ռ�任����
struct FObjectTransformation
{
	FObjectTransformation();

	XMFLOAT4X4 World;

	static XMFLOAT4X4 IdentityMatrix4x4();
};

class CMesh : public CCoreMinimalObject, public IRenderingInterface
{
public:
	CMesh();
	virtual void Init();
	virtual void BuildMesh(const FMeshRenderingData* InRenderingData);

	virtual void PreDraw(float DeltaTime);
	virtual void Draw(float DeltaTime);
	virtual void PostDraw(float DeltaTime);

	/// <summary>
	/// �������� ���ز�ͬ���͵�Mesh����ʵ�� cube sphere capsule clyinder
	/// </summary>
	static CMesh* CreateMesh(const FMeshRenderingData* InRenderingData);

	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView();
	D3D12_INDEX_BUFFER_VIEW GetIndexBufferView();
	
protected:
	ComPtr<ID3DBlob> CPUVertexBufferPtr;
	ComPtr<ID3DBlob> CPUIndexBufferPtr;

	ComPtr<ID3D12Resource> GPUVertexBufferPtr;
	ComPtr<ID3D12Resource> GPUIndexBufferPtr;

	ComPtr<ID3D12Resource> VertexBufferTempPtr;// Upload����
	ComPtr<ID3D12Resource> IndexBufferTempPtr;// Upload����

	ComPtr<ID3D12RootSignature> RootSignature;// shader���õ�����Դ��
	ComPtr<ID3D12DescriptorHeap> CBVHeap;
	shared_ptr<FRenderingResourcesUpdate> ObjectConstants;// ConstantBuffer����������

	ComPtr<ID3D12PipelineState> PSO;

	FShader VertexShader;
	FShader PixelShader;

	vector<D3D12_INPUT_ELEMENT_DESC> InputElementDesc;// ������ɫ���������������ЩԪ�ء�ÿ��Ԫ�ص�����
protected:
	UINT VertexSizeInBytes;
	UINT VertexStrideInBytes;

	UINT IndexSizeInBytes;
	DXGI_FORMAT IndexFormat;
	UINT IndexSize;

	XMFLOAT4X4 WorldMatrix;
	XMFLOAT4X4 ViewMatrix;
	XMFLOAT4X4 ProjectionMatrix;
};