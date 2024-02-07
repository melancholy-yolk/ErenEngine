#pragma once
#include "../../Rendering/Core/Rendering.h"
#include "MeshType.h"
#include "../../Shader/Core/Shader.h"

// 物体空间变换数据
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
	/// 工厂方法 返回不同类型的Mesh子类实例 cube sphere capsule clyinder
	/// </summary>
	static CMesh* CreateMesh(const FMeshRenderingData* InRenderingData);

	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView();
	D3D12_INDEX_BUFFER_VIEW GetIndexBufferView();
	
protected:
	ComPtr<ID3DBlob> CPUVertexBufferPtr;
	ComPtr<ID3DBlob> CPUIndexBufferPtr;

	ComPtr<ID3D12Resource> GPUVertexBufferPtr;
	ComPtr<ID3D12Resource> GPUIndexBufferPtr;

	ComPtr<ID3D12Resource> VertexBufferTempPtr;// Upload缓冲
	ComPtr<ID3D12Resource> IndexBufferTempPtr;// Upload缓冲

	ComPtr<ID3D12RootSignature> RootSignature;// shader中用到的资源绑定
	ComPtr<ID3D12DescriptorHeap> CBVHeap;
	shared_ptr<FRenderingResourcesUpdate> ObjectConstants;// ConstantBuffer常量缓冲区

	ComPtr<ID3D12PipelineState> PSO;

	FShader VertexShader;
	FShader PixelShader;

	vector<D3D12_INPUT_ELEMENT_DESC> InputElementDesc;// 顶点着色器单个顶点包含那些元素、每个元素的属性
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