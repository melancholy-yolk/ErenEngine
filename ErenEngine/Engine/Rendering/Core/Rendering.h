#pragma once

#include "../../Core/CoreObject/GuidInterface.h"
#include "../../Core/Engine.h"

#if defined(_WIN32)
#include "../../Platform/Windows/WindowsEngine.h"
#else

#endif

class IRenderingInterface : public IGuidInterface
{

#if defined(_WIN32)
	friend class CWindowsEngine;
#endif

public:
	IRenderingInterface();
	virtual ~IRenderingInterface();

	virtual void Init();

	virtual void PreDraw(float DeltaTime);
	virtual void Draw(float DeltaTime);
	virtual void PostDraw(float DeltaTime);

protected:
	/// <summary>
	/// 创建 VertexBuffer对应的GPU资源
	/// </summary>
	ComPtr<ID3D12Resource> ConstructDefaultBuffer(ComPtr<ID3D12Resource>& OutTempBuffer, const void* InData, UINT64 InDataSize);

protected:

#if defined(_WIN32)
	CWindowsEngine* GetEngine();
#else
	FEngine* GetEngine();
#endif

	ComPtr<ID3D12Device> GetD3DDevice();
	ComPtr<ID3D12GraphicsCommandList> GetGraphicsCommandList();
	ComPtr<ID3D12CommandAllocator> GetCommandAllocator();

private:
	static vector<IRenderingInterface*>  RenderingInterfaces;
};

class FRenderingResourcesUpdate : public enable_shared_from_this<FRenderingResourcesUpdate>
{
public:
	FRenderingResourcesUpdate();
	~FRenderingResourcesUpdate();

	void Init(ID3D12Device* InDevice, UINT InElementSize, UINT InElementCount);

	void Update(int Index, const void* InData);

	UINT GetConstantBufferSize(UINT InTypeSize);
	UINT GetConstantBufferSize();

	ID3D12Resource* GetBuffer() { return UploadBuffer.Get(); }
private:
	ComPtr<ID3D12Resource> UploadBuffer;
	UINT ElementSize;
	BYTE* Data;
};