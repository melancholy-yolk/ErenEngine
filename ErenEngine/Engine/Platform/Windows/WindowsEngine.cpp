#include "WindowsEngine.h"
#include "../../Debug/EngineDebug.h"
#include "../../Config/EngineRenderConfig.h"
#include "../../Rendering/Core/Rendering.h"
#include "../../Mesh/BoxMesh.h"
#include "../../Mesh/SphereMesh.h"
#include "../../Mesh/CylinderMesh.h"
#include "../../Mesh/CustomMesh.h"
#include "../../Mesh/ConeMesh.h"
#include "../../Mesh/PlaneMesh.h"
#include "../../Core/World.h"

#if defined(_WIN32)
#include "WindowsMessageProcessing.h"

CWindowsEngine::CWindowsEngine()
	:CurrentFenceIndex(0),
	M4XQualityLevels(0),
	bMSAA4XEnabled(false),
	BackBufferFormat(DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM),
	DepthStencilFormat(DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT),
	CurrentSwapBufferIndex(0)
{
	for (int i = 0; i < FEngineRenderConfig::GetEngineRenderConfig()->SwapChainBufferCount; i++)
	{
		SwapChainBuffer.push_back(ComPtr<ID3D12Resource>());
	}

	bTick = false;
}

CWindowsEngine::~CWindowsEngine()
{

}

int CWindowsEngine::PreInit(FWinMainCommandParameters InParameters)
{
	//��ʼ����־ϵͳ
	const char LogPath[] = "../log";
	init_log_system(LogPath);
	ENGINE_LOG("Log System Init.");

	// Parse CommandLine Parameters

	
	ENGINE_LOG("Engine PreInit Complete.");
	return 0;
}

int CWindowsEngine::Init(FWinMainCommandParameters InParameters)
{
	//������
	if (!InitWindows(InParameters))
	{
		ENGINE_LOG_ERROR("Engine Init Windows Fail.");
		return 1;
	}

	InitDirect3D();
	PostInitDirect3D();

	CWorld* World = CreateObject<CWorld>(new CWorld());

	ENGINE_LOG("Engine Init Complete.");
	return 0;
}

int CWindowsEngine::PostInit()
{
	ANALYSIS_HRESULT(GraphicsCommandList->Reset(CommandAllocator.Get(), NULL););
	{
		// ����Meshʵ�� ���򻯴�����Mesh ����RHIResource
		// �����������ڴ���ֻ����һ�� ���ǿ����ж����������������ݵĿ���Ⱦ���� ���ڿ���Ⱦ�����ύ��ȾDrawcall ʵ������Ⱦ ��ͬ��RenderPass���Ը����ύ��ͬ����Ⱦ����
		// ������������б�����������VertexBuffer��CPU�ϴ���GPU
		/*CBoxMesh* BoxMesh = CBoxMesh::CreateMesh(5.f, 3.f, 1.f);
		CSphereMesh* SphereMesh = CSphereMesh::CreateMesh(2, 20, 20);
		CCylinderMesh* CylinderMesh = CCylinderMesh::CreateMesh(2.f, 2.f, 5.f, 20, 20);
		CConeMesh* ConeMesh = CConeMesh::CreateMesh(2.f, 5.f, 10, 10);
		CPlaneMesh* PlaneMesh = CPlaneMesh::CreateMesh(10, 10, 10, 10);*/
		CCustomMesh* CustomMesh = CCustomMesh::CreateMesh("../ErenEngine/Asset/Test.obj");

		for (auto& Tmp : GObjects)
		{
			Tmp->BeginInit();
		}
	}
	ANALYSIS_HRESULT(GraphicsCommandList->Close());

	ID3D12CommandList* CommandList[] = { GraphicsCommandList.Get() };
	CommandQueue->ExecuteCommandLists(_countof(CommandList), CommandList);

	WaitGPUCommandQueueComplete();

	ENGINE_LOG("Engine PostInit Complete.");
	return 0;
}

void CWindowsEngine::Tick(float DeltaTime)
{
	// Logic Tick
	for (CCoreMinimalObject*& Obj : GObjects)
	{
		if (Obj->IsTick())
		{
			Obj->Tick(DeltaTime);
		}
	}

	// Render Tick
	// ������Ⱦ����¼������ڴ棬Ϊ�µ�һ֡��׼��
	ANALYSIS_HRESULT(CommandAllocator->Reset());

	for (auto& Tmp : IRenderingInterface::RenderingInterfaces)
	{
		Tmp->PreDraw(DeltaTime);
	}

	//���������б� ���ÿ����ͬ��PSO ����Ҫʹ�ö�Ӧ��PSO��reset
	//GraphicsCommandList->Reset(CommandAllocator.Get(), NULL);

	//ָ��һ��GPU��Դ ת����״̬ ����RTΪ��д״̬
	CD3DX12_RESOURCE_BARRIER Present2RenderTargetBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
		GetCurrentSwapBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, 
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	GraphicsCommandList->ResourceBarrier(1, &Present2RenderTargetBarrier);

	GraphicsCommandList->RSSetViewports(1, &ViewportInfo);//�����ӿ�
	GraphicsCommandList->RSSetScissorRects(1, &ViewportRect);//�ӿڲü�

	GraphicsCommandList->ClearRenderTargetView(GetCurrentSwapBufferView(), DirectX::Colors::Black, 0, nullptr);// Clear RTV
	GraphicsCommandList->ClearDepthStencilView(GetCurrentDepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.f, 0, 0, NULL);// Clear DSV

	D3D12_CPU_DESCRIPTOR_HANDLE SwapBufferView = GetCurrentSwapBufferView();
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView = GetCurrentDepthStencilView();
	GraphicsCommandList->OMSetRenderTargets(1, &SwapBufferView, true, &DepthStencilView);

	//�����Ⱦ����
	for (auto &Tmp : IRenderingInterface::RenderingInterfaces)
	{
		Tmp->Draw(DeltaTime);
		Tmp->PostDraw(DeltaTime);
	}

	//ָ��һ��GPU��Դ ת����״̬ ����RTΪ����д״̬
	CD3DX12_RESOURCE_BARRIER RenderTarget2PresentBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
		GetCurrentSwapBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, 
		D3D12_RESOURCE_STATE_PRESENT
	);
	GraphicsCommandList->ResourceBarrier(1, &RenderTarget2PresentBarrier);

	//��Ⱦ����¼�����
	ANALYSIS_HRESULT(GraphicsCommandList->Close());

	//�ύ����
	ID3D12CommandList* CommandListArr[] = { GraphicsCommandList.Get() };
	CommandQueue->ExecuteCommandLists(_countof(CommandListArr), CommandListArr);

	//�л�������
	ANALYSIS_HRESULT(SwapChain->Present(0, 0));
	CurrentSwapBufferIndex = !(bool)CurrentSwapBufferIndex;

	//CPU�ȴ�GPU��Ⱦ��� �ȴ��¼�֪ͨ
	WaitGPUCommandQueueComplete();
}

int CWindowsEngine::PreExit()
{


	ENGINE_LOG("Engine Pre Exit Complete.");
	return 0;
}

int CWindowsEngine::Exit()
{


	ENGINE_LOG("Engine Exit Complete.");
	return 0;
}

int CWindowsEngine::PostExit()
{
	FEngineRenderConfig::Destroy();

	ENGINE_LOG("Engine Post Exit Complete.");
	return 0;
}

ID3D12Resource* CWindowsEngine::GetCurrentSwapBuffer() const
{
	return SwapChainBuffer[CurrentSwapBufferIndex].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE CWindowsEngine::GetCurrentSwapBufferView() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(RTVHeap->GetCPUDescriptorHandleForHeapStart(), CurrentSwapBufferIndex, RTVDescriptorSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE CWindowsEngine::GetCurrentDepthStencilView() const
{
	return DSVHeap->GetCPUDescriptorHandleForHeapStart();
}

UINT CWindowsEngine::GetDXGISampleCount() const
{
	return bMSAA4XEnabled ? 4 : 1;
}

UINT CWindowsEngine::GetDXGISampleQuality() const
{
	return bMSAA4XEnabled ? (M4XQualityLevels - 1) : 0;
}

void CWindowsEngine::WaitGPUCommandQueueComplete()
{
	CurrentFenceIndex++;

	//��GPU�����µĸ���� �ȴ�GPU�������ź�
	ANALYSIS_HRESULT(CommandQueue->Signal(Fence.Get(), CurrentFenceIndex));

	if (Fence->GetCompletedValue() < CurrentFenceIndex)
	{
		HANDLE EventEx = CreateEventEx(NULL, NULL, 0, EVENT_ALL_ACCESS);
		ANALYSIS_HRESULT(Fence->SetEventOnCompletion(CurrentFenceIndex, EventEx));

		// �ȴ�GPU �������߳�
		WaitForSingleObject(EventEx, INFINITE);
		CloseHandle(EventEx);
	}
}

bool CWindowsEngine::InitWindows(FWinMainCommandParameters InParameters)
{
	WNDCLASSEX WindowsClass;
	WindowsClass.cbSize = sizeof(WNDCLASSEX);
	WindowsClass.cbClsExtra = 0;
	WindowsClass.cbWndExtra = 0;
	WindowsClass.hbrBackground = nullptr;
	WindowsClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WindowsClass.hIcon = nullptr;
	WindowsClass.hIconSm = NULL;
	WindowsClass.hInstance = InParameters.HInstance;
	WindowsClass.lpszClassName = L"ErenEngine";
	WindowsClass.lpszMenuName = nullptr;
	WindowsClass.style = CS_VREDRAW | CS_HREDRAW;
	WindowsClass.lpfnWndProc = EngineWindowProc;

	ATOM RegisterAtom = RegisterClassEx(&WindowsClass);
	if (!RegisterAtom)
	{
		ENGINE_LOG_ERROR("Window Register Fail.");
		MessageBox(NULL, L"Window Register Fail,", L"Error", MB_OK);
	}

	RECT Rect = {
		0, 
		0, 
		FEngineRenderConfig::GetEngineRenderConfig()->ScreenWidth, 
		FEngineRenderConfig::GetEngineRenderConfig()->ScreenHeight
	};
	AdjustWindowRect(&Rect, WS_OVERLAPPEDWINDOW, NULL);
	int WindowWidth = Rect.right - Rect.left;
	int WindowHeight = Rect.bottom - Rect.top;
	MainWindowsHandle = CreateWindowEx(
		NULL,
		L"ErenEngine",
		L"Eren Engine",
		WS_OVERLAPPEDWINDOW,
		100, 100,
		WindowWidth, WindowHeight,
		NULL,
		nullptr,
		InParameters.HInstance,
		NULL);
	if (!MainWindowsHandle)
	{
		ENGINE_LOG_ERROR("Create Window Fail.");
		MessageBox(0, L"Create Window Fail.", 0, 0);
		return false;
	}

	ShowWindow(MainWindowsHandle, SW_SHOW);
	UpdateWindow(MainWindowsHandle);

	ENGINE_LOG("Engine Init Windows Complete.");

	return true;
}
bool CWindowsEngine::InitDirect3D()
{
	// Debug
	ComPtr<ID3D12Debug> D3D12Debug;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&D3D12Debug))))
	{
		D3D12Debug->EnableDebugLayer();
	}

	// E_UNEXPECTED
	// E_NOTIMPL
	// E_OUTOFMEMORY
	// E_INVALIDARG
	// E_NOINTERFACE
	// E_POINTER
	// E_HANDLE
	// E_ABORT
	// E_FAIL
	// E_ACCESSDENIED
	ANALYSIS_HRESULT(CreateDXGIFactory1(IID_PPV_ARGS(&DXGIFactory)));

	// Create D3D12Device
	HRESULT D3dDeviceResult = D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&D3DDevice));
	if (FAILED(D3dDeviceResult))
	{
		//����Ӳ����Ⱦ����ʧ�ܣ����Դ�������Ⱦ������
		//warp
		ComPtr<IDXGIAdapter> WARPAdapter;
		ANALYSIS_HRESULT(DXGIFactory->EnumWarpAdapter(IID_PPV_ARGS(&WARPAdapter)));

		ANALYSIS_HRESULT(D3D12CreateDevice(WARPAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&D3DDevice)));
	}

	// Create D3D12Fence
	ANALYSIS_HRESULT(D3DDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence)));

	// CreateCommandQueue
	D3D12_COMMAND_QUEUE_DESC QueueDesc = {};
	QueueDesc.Type = D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT;
	QueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAGS::D3D12_COMMAND_QUEUE_FLAG_NONE;
	ANALYSIS_HRESULT(D3DDevice->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(&CommandQueue)));

	// CreateCommandAllocator
	ANALYSIS_HRESULT(D3DDevice->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(CommandAllocator.GetAddressOf())
	));

	// CreateCommandList
	ANALYSIS_HRESULT(D3DDevice->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT,
		CommandAllocator.Get(),
		NULL,
		IID_PPV_ARGS(GraphicsCommandList.GetAddressOf())));

	ANALYSIS_HRESULT(GraphicsCommandList->Close());

	// MSAA Feature CheckFeatureSupport
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS QualityLevels;
	QualityLevels.Format = BackBufferFormat;
	QualityLevels.SampleCount = 4;
	QualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVEL_FLAGS::D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	QualityLevels.NumQualityLevels = 0;

	ANALYSIS_HRESULT(D3DDevice->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&QualityLevels,
		sizeof(QualityLevels)
	));

	M4XQualityLevels = QualityLevels.NumQualityLevels;

	// ������
	SwapChain.Reset();
	DXGI_SWAP_CHAIN_DESC SwapChainDesc;
	SwapChainDesc.BufferDesc.Width = FEngineRenderConfig::GetEngineRenderConfig()->ScreenWidth;
	SwapChainDesc.BufferDesc.Height = FEngineRenderConfig::GetEngineRenderConfig()->ScreenHeight;
	SwapChainDesc.BufferDesc.RefreshRate.Numerator = FEngineRenderConfig::GetEngineRenderConfig()->RefreshRate;
	SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	SwapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER::DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	SwapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	SwapChainDesc.BufferCount = FEngineRenderConfig::GetEngineRenderConfig()->SwapChainBufferCount;
	
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.OutputWindow = MainWindowsHandle;
	SwapChainDesc.Windowed = true;
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; 
	SwapChainDesc.BufferDesc.Format = BackBufferFormat;

	SwapChainDesc.SampleDesc.Count = GetDXGISampleCount();
	SwapChainDesc.SampleDesc.Quality = GetDXGISampleQuality();

	HRESULT CreateSwapChainResult = DXGIFactory->CreateSwapChain(CommandQueue.Get(), &SwapChainDesc, SwapChain.GetAddressOf());
	if (FAILED(CreateSwapChainResult))
	{
		ENGINE_LOG_ERROR("Error = %i", (int)CreateSwapChainResult);
	}

	// RenderTargetView CreateDescriptorHeap
	D3D12_DESCRIPTOR_HEAP_DESC RTVDescriptorHeapDesc;
	RTVDescriptorHeapDesc.NumDescriptors = FEngineRenderConfig::GetEngineRenderConfig()->SwapChainBufferCount;
	RTVDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	RTVDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	RTVDescriptorHeapDesc.NodeMask = 0;
	ANALYSIS_HRESULT(D3DDevice->CreateDescriptorHeap(
		&RTVDescriptorHeapDesc, 
		IID_PPV_ARGS(RTVHeap.GetAddressOf()))
	);

	// DepthStencilView CreateDescriptorHeap
	D3D12_DESCRIPTOR_HEAP_DESC DSVDescriptorHeapDesc;
	DSVDescriptorHeapDesc.NumDescriptors = 1;
	DSVDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	DSVDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DSVDescriptorHeapDesc.NodeMask = 0;
	ANALYSIS_HRESULT(D3DDevice->CreateDescriptorHeap(
		&DSVDescriptorHeapDesc, 
		IID_PPV_ARGS(DSVHeap.GetAddressOf()))
	);

	return true;
}
void CWindowsEngine::PostInitDirect3D()
{
	// �ȴ�DirectX��һЩ���󴴽����
	WaitGPUCommandQueueComplete();

	ANALYSIS_HRESULT(GraphicsCommandList->Reset(CommandAllocator.Get(), NULL));

	// ����GPU�Դ��ϵĽ�����ʹ�õ�����RenderTarget���������ΪOpenGL�е�FrameBuffer��
	for (int i = 0; i < FEngineRenderConfig::GetEngineRenderConfig()->SwapChainBufferCount; i++)
	{
		SwapChainBuffer[i].Reset();
	}
	DepthStencilBuffer.Reset();

	// �����ڲ�Ӧ�û�Ϊ���Ǵ�����������ʹ�õ� ID3D12Resource
	SwapChain->ResizeBuffers(
		FEngineRenderConfig::GetEngineRenderConfig()->SwapChainBufferCount,
		FEngineRenderConfig::GetEngineRenderConfig()->ScreenWidth,
		FEngineRenderConfig::GetEngineRenderConfig()->ScreenHeight,
		BackBufferFormat,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
	);

	// �� DescriptorHeap �д��� RTV
	// ����������һ���������ڴ� ���ڴ洢������
	// ����������һ��������������ռ�õ��ڴ��С ���������
	RTVDescriptorSize = D3DDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	//D3D12_CPU_DESCRIPTOR_HANDLE HeapHandle = RTVHeap->GetCPUDescriptorHandleForHeapStart();
	CD3DX12_CPU_DESCRIPTOR_HANDLE HeapHandle(RTVHeap->GetCPUDescriptorHandleForHeapStart());
	//HeapHandle.ptr = 0;

	for (UINT i = 0; i < FEngineRenderConfig::GetEngineRenderConfig()->SwapChainBufferCount; i++)
	{
		SwapChain->GetBuffer(i, IID_PPV_ARGS(&SwapChainBuffer[i]));
		D3DDevice->CreateRenderTargetView(SwapChainBuffer[i].Get(), nullptr, HeapHandle);
		//HeapHandle.ptr += RTVDescriptorSize;
		HeapHandle.Offset(1, RTVDescriptorSize);
	}

	// �������ģ�建������Դ
	D3D12_RESOURCE_DESC ResourceDesc;
	ResourceDesc.Width = FEngineRenderConfig::GetEngineRenderConfig()->ScreenWidth;
	ResourceDesc.Height = FEngineRenderConfig::GetEngineRenderConfig()->ScreenHeight;
	ResourceDesc.Alignment = 0;
	ResourceDesc.MipLevels = 1;
	ResourceDesc.DepthOrArraySize = 1;
	ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

	ResourceDesc.SampleDesc.Count = bMSAA4XEnabled ? 4 : 1;
	ResourceDesc.SampleDesc.Quality = bMSAA4XEnabled ? (M4XQualityLevels - 1) : 0;
	ResourceDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	ResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

	D3D12_CLEAR_VALUE ClearValue;
	ClearValue.DepthStencil.Depth = 1.f;
	ClearValue.DepthStencil.Stencil = 0;
	ClearValue.Format = DepthStencilFormat;

	CD3DX12_HEAP_PROPERTIES HeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	D3DDevice->CreateCommittedResource(
		&HeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&ResourceDesc,
		D3D12_RESOURCE_STATE_COMMON,
		&ClearValue,
		IID_PPV_ARGS(DepthStencilBuffer.GetAddressOf())
	);

	// DepthStencilView ��DSVHeap�д���DSV
	D3D12_DEPTH_STENCIL_VIEW_DESC DSVDesc;
	DSVDesc.Format = DepthStencilFormat;
	DSVDesc.Texture2D.MipSlice = 0;
	DSVDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	DSVDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3DDevice->CreateDepthStencilView(DepthStencilBuffer.Get(), &DSVDesc, DSVHeap->GetCPUDescriptorHandleForHeapStart());

	//���GPU��Դ(һ���Դ�)�Ķ�д״̬ ��ֹ���ֻ�δ�����Ѿ����޸ĵ����
	//��Դͬ�������洢��ʽ
	CD3DX12_RESOURCE_BARRIER Barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		DepthStencilBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_DEPTH_WRITE
	);
	GraphicsCommandList->ResourceBarrier(1, &Barrier);

	GraphicsCommandList->Close();

	ID3D12CommandList* CommandList[] = { GraphicsCommandList.Get() };
	CommandQueue->ExecuteCommandLists(_countof(CommandList), CommandList);

	ViewportInfo.TopLeftX = 0;
	ViewportInfo.TopLeftY = 0;
	ViewportInfo.Width = FEngineRenderConfig::GetEngineRenderConfig()->ScreenWidth;
	ViewportInfo.Height = FEngineRenderConfig::GetEngineRenderConfig()->ScreenHeight;
	ViewportInfo.MinDepth = 0.f;
	ViewportInfo.MaxDepth = 1.f;

	ViewportRect.top = 0;
	ViewportRect.left = 0;
	ViewportRect.right = FEngineRenderConfig::GetEngineRenderConfig()->ScreenWidth;
	ViewportRect.bottom = FEngineRenderConfig::GetEngineRenderConfig()->ScreenHeight;

	WaitGPUCommandQueueComplete();
}
#endif