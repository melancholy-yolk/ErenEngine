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
	//初始化日志系统
	const char LogPath[] = "../log";
	init_log_system(LogPath);
	ENGINE_LOG("Log System Init.");

	// Parse CommandLine Parameters

	
	ENGINE_LOG("Engine PreInit Complete.");
	return 0;
}

int CWindowsEngine::Init(FWinMainCommandParameters InParameters)
{
	//处理窗口
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
		// 创建Mesh实例 程序化创建的Mesh 创建RHIResource
		// 网格数据在内存中只存在一份 但是可以有多个引用这个网格数据的可渲染对象 基于可渲染对象提交渲染Drawcall 实例化渲染 不同的RenderPass可以负责提交不同的渲染需求
		// 这里会向命令列表中填充命令：将VertexBuffer从CPU上传到GPU
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
	// 重置渲染命令录入相关内存，为新的一帧做准备
	ANALYSIS_HRESULT(CommandAllocator->Reset());

	for (auto& Tmp : IRenderingInterface::RenderingInterfaces)
	{
		Tmp->PreDraw(DeltaTime);
	}

	//重置命令列表 针对每个不同的PSO 都需要使用对应的PSO来reset
	//GraphicsCommandList->Reset(CommandAllocator.Get(), NULL);

	//指定一个GPU资源 转换其状态 设置RT为可写状态
	CD3DX12_RESOURCE_BARRIER Present2RenderTargetBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
		GetCurrentSwapBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, 
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	GraphicsCommandList->ResourceBarrier(1, &Present2RenderTargetBarrier);

	GraphicsCommandList->RSSetViewports(1, &ViewportInfo);//设置视口
	GraphicsCommandList->RSSetScissorRects(1, &ViewportRect);//视口裁剪

	GraphicsCommandList->ClearRenderTargetView(GetCurrentSwapBufferView(), DirectX::Colors::Black, 0, nullptr);// Clear RTV
	GraphicsCommandList->ClearDepthStencilView(GetCurrentDepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.f, 0, 0, NULL);// Clear DSV

	D3D12_CPU_DESCRIPTOR_HANDLE SwapBufferView = GetCurrentSwapBufferView();
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView = GetCurrentDepthStencilView();
	GraphicsCommandList->OMSetRenderTargets(1, &SwapBufferView, true, &DepthStencilView);

	//填充渲染命令
	for (auto &Tmp : IRenderingInterface::RenderingInterfaces)
	{
		Tmp->Draw(DeltaTime);
		Tmp->PostDraw(DeltaTime);
	}

	//指定一个GPU资源 转换其状态 设置RT为不可写状态
	CD3DX12_RESOURCE_BARRIER RenderTarget2PresentBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
		GetCurrentSwapBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, 
		D3D12_RESOURCE_STATE_PRESENT
	);
	GraphicsCommandList->ResourceBarrier(1, &RenderTarget2PresentBarrier);

	//渲染命令录入完成
	ANALYSIS_HRESULT(GraphicsCommandList->Close());

	//提交命令
	ID3D12CommandList* CommandListArr[] = { GraphicsCommandList.Get() };
	CommandQueue->ExecuteCommandLists(_countof(CommandListArr), CommandListArr);

	//切换交换链
	ANALYSIS_HRESULT(SwapChain->Present(0, 0));
	CurrentSwapBufferIndex = !(bool)CurrentSwapBufferIndex;

	//CPU等待GPU渲染完成 等待事件通知
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

	//向GPU设置新的隔离点 等待GPU处理完信号
	ANALYSIS_HRESULT(CommandQueue->Signal(Fence.Get(), CurrentFenceIndex));

	if (Fence->GetCompletedValue() < CurrentFenceIndex)
	{
		HANDLE EventEx = CreateEventEx(NULL, NULL, 0, EVENT_ALL_ACCESS);
		ANALYSIS_HRESULT(Fence->SetEventOnCompletion(CurrentFenceIndex, EventEx));

		// 等待GPU 阻塞主线程
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
		//创建硬件渲染驱动失败，尝试创建软渲染器驱动
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

	// 交换链
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
	// 等待DirectX的一些对象创建完成
	WaitGPUCommandQueueComplete();

	ANALYSIS_HRESULT(GraphicsCommandList->Reset(CommandAllocator.Get(), NULL));

	// 创建GPU显存上的交换链使用的两个RenderTarget（可以理解为OpenGL中的FrameBuffer）
	for (int i = 0; i < FEngineRenderConfig::GetEngineRenderConfig()->SwapChainBufferCount; i++)
	{
		SwapChainBuffer[i].Reset();
	}
	DepthStencilBuffer.Reset();

	// 这里内部应该会为我们创建出交换链使用的 ID3D12Resource
	SwapChain->ResizeBuffers(
		FEngineRenderConfig::GetEngineRenderConfig()->SwapChainBufferCount,
		FEngineRenderConfig::GetEngineRenderConfig()->ScreenWidth,
		FEngineRenderConfig::GetEngineRenderConfig()->ScreenHeight,
		BackBufferFormat,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
	);

	// 在 DescriptorHeap 中创建 RTV
	// 描述符堆是一块连续的内存 用于存储描述符
	// 描述符堆中一个描述符对象所占用的内存大小 描述符句柄
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

	// 创建深度模板缓冲区资源
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

	// DepthStencilView 在DSVHeap中创建DSV
	D3D12_DEPTH_STENCIL_VIEW_DESC DSVDesc;
	DSVDesc.Format = DepthStencilFormat;
	DSVDesc.Texture2D.MipSlice = 0;
	DSVDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	DSVDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3DDevice->CreateDepthStencilView(DepthStencilBuffer.Get(), &DSVDesc, DSVHeap->GetCPUDescriptorHandleForHeapStart());

	//标记GPU资源(一块显存)的读写状态 防止出现还未读完已经被修改的情况
	//资源同步、表达、存储方式
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