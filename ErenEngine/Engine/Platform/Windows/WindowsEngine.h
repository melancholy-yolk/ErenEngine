#pragma once

#if defined(_WIN32)
#include "../../Core/Engine.h"

class CWindowsEngine : public CEngine
{
	friend class IRenderingInterface;// ����Ⱦ��λ
public:
	CWindowsEngine();
	~CWindowsEngine();

	virtual int PreInit(FWinMainCommandParameters InParameters);
	virtual int Init(FWinMainCommandParameters InParameters);
	virtual int PostInit();

	virtual void Tick(float DeltaTime);

	virtual int PreExit();
	virtual int Exit();
	virtual int PostExit();

public:
	ID3D12Resource* GetCurrentSwapBuffer() const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentSwapBufferView() const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentDepthStencilView() const;

public:
	DXGI_FORMAT GetBackBufferFormat() const { return BackBufferFormat; };
	DXGI_FORMAT GetDepthStencilFormat() const { return DepthStencilFormat; };
	UINT GetDXGISampleCount() const;
	UINT GetDXGISampleQuality() const;

protected:
	void WaitGPUCommandQueueComplete();

private:
	bool InitWindows(FWinMainCommandParameters InParameters);

	bool InitDirect3D();
	void PostInitDirect3D();

protected:
	UINT64 CurrentFenceIndex;
	int CurrentSwapBufferIndex;

	ComPtr<IDXGIFactory4> DXGIFactory;
	ComPtr<ID3D12Device> D3DDevice;//��������������������б�������С�Fence����Դ���ܵ�״̬����PSO���ѡ���ǩ��������������
	ComPtr<ID3D12Fence> Fence;

	ComPtr<ID3D12CommandQueue> CommandQueue;
	ComPtr<ID3D12CommandAllocator> CommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> GraphicsCommandList;

	ComPtr<IDXGISwapChain> SwapChain;

	ComPtr<ID3D12DescriptorHeap> RTVHeap;
	ComPtr<ID3D12DescriptorHeap> DSVHeap;

	vector<ComPtr<ID3D12Resource>> SwapChainBuffer;
	ComPtr<ID3D12Resource> DepthStencilBuffer;

	//��Ļ�ӿ����
	D3D12_VIEWPORT ViewportInfo;
	D3D12_RECT ViewportRect;
protected:
	HWND MainWindowsHandle; // Windows���ھ��
	UINT M4XQualityLevels; // MSAA�����ȼ�
	bool bMSAA4XEnabled; // MSAA����
	DXGI_FORMAT BackBufferFormat;
	DXGI_FORMAT DepthStencilFormat;
	UINT RTVDescriptorSize;
};
#endif