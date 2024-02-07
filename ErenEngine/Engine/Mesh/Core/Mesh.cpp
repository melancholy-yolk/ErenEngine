#include "Mesh.h"
#include "../../Config/EngineRenderConfig.h"

CMesh::CMesh()
	: VertexSizeInBytes(0)
	, VertexStrideInBytes(0)
	, IndexSizeInBytes(0)
	, IndexFormat(DXGI_FORMAT_R16_UINT)
	, IndexSize(0)
	, WorldMatrix(FObjectTransformation::IdentityMatrix4x4())
	, ViewMatrix(FObjectTransformation::IdentityMatrix4x4())
	, ProjectionMatrix(FObjectTransformation::IdentityMatrix4x4())
{
	
}

const float PI = 3.1415926535f;

void CMesh::Init()
{
	// ת����[-1, 1]^3�Ŀռ�NDC ֮������ӿڱ任 * 0.5 + 0.5 ת������Ļ�ռ����� zֵ��Ϊ���ֵ
	float FovAngleY = 0.25f * PI;
	float AspectRatio = (float)FEngineRenderConfig::GetEngineRenderConfig()->ScreenWidth / (float)FEngineRenderConfig::GetEngineRenderConfig()->ScreenHeight;
	float NearZ = 1.0f;
	float FarZ = 1000.0f;
	XMMATRIX Project = XMMatrixPerspectiveFovLH(FovAngleY, AspectRatio, NearZ, FarZ);
	XMStoreFloat4x4(&ProjectionMatrix, Project);
}

void CMesh::BuildMesh(const FMeshRenderingData* InRenderingData)
{
	// ==================== BEGIN CBV ====================
	// ����CBV��������
	D3D12_DESCRIPTOR_HEAP_DESC CBVDescriptorHeapDesc;
	CBVDescriptorHeapDesc.NumDescriptors = 1;
	CBVDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	CBVDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	CBVDescriptorHeapDesc.NodeMask = 0;
	GetD3DDevice()->CreateDescriptorHeap(&CBVDescriptorHeapDesc, IID_PPV_ARGS(&CBVHeap));

	// ����������������Դ ConstantBuffer ID3D12Resource
	ObjectConstants = make_shared<FRenderingResourcesUpdate>();
	ObjectConstants->Init(GetD3DDevice().Get(), sizeof(FObjectTransformation), 1);
	D3D12_GPU_VIRTUAL_ADDRESS OCBAddress = ObjectConstants.get()->GetBuffer()->GetGPUVirtualAddress();

	// ����������������ͼ CBV
	D3D12_CONSTANT_BUFFER_VIEW_DESC CBVDesc;
	CBVDesc.BufferLocation = OCBAddress;
	CBVDesc.SizeInBytes = ObjectConstants->GetConstantBufferSize();

	GetD3DDevice()->CreateConstantBufferView(
		&CBVDesc, 
		CBVHeap->GetCPUDescriptorHandleForHeapStart());
	// ==================== END CBV ====================
	




	// ==================== BEGIN Root Signature ====================
	CD3DX12_ROOT_PARAMETER RootParam[1];

	CD3DX12_DESCRIPTOR_RANGE DescriptorRangeCBV;
	DescriptorRangeCBV.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);

	RootParam[0].InitAsDescriptorTable(1, &DescriptorRangeCBV);

	CD3DX12_ROOT_SIGNATURE_DESC RootSignatureDesc(
		1, 
		RootParam, 
		0, 
		nullptr, 
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> SerializeRootSignature;
	ComPtr<ID3DBlob> ErrorBlob;
	D3D12SerializeRootSignature(
		&RootSignatureDesc, 
		D3D_ROOT_SIGNATURE_VERSION_1, 
		SerializeRootSignature.GetAddressOf(), 
		ErrorBlob.GetAddressOf());

	if (ErrorBlob)
	{
		ENGINE_LOG_ERROR("%s", (char*)ErrorBlob->GetBufferPointer());
	}

	// ˼������ǩ��û���������ѡ���ͼ
	GetD3DDevice()->CreateRootSignature(
		0, 
		SerializeRootSignature->GetBufferPointer(),
		SerializeRootSignature->GetBufferSize(),
		IID_PPV_ARGS(&RootSignature));
	// ==================== END Root Signature ====================





	// ==================== BEGIN Shader ====================
	VertexShader.BuildShaders(L"F:\\ErenEngine\\ErenEngine\\Shader\\Hello.hlsl", "VertexShaderMain", "vs_5_0");
	PixelShader.BuildShaders(L"F:\\ErenEngine\\ErenEngine\\Shader\\Hello.hlsl", "PixelShaderMain", "ps_5_0");

	D3D12_INPUT_ELEMENT_DESC InputElementDescTemp;
	D3D12_INPUT_LAYOUT_DESC InputLayoutDesc;
	InputElementDesc =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};
	// ==================== END Shader ====================





	// ==================== BEGIN VertexBuffer IndexBuffer ====================
	// ���� ���㻺���� ����������
	VertexStrideInBytes = sizeof(FVertex);
	IndexSize = InRenderingData->IndexData.size();

	VertexSizeInBytes = InRenderingData->VertexData.size() * VertexStrideInBytes;
	IndexSizeInBytes = InRenderingData->IndexData.size() * sizeof(uint16_t);

	ANALYSIS_HRESULT(D3DCreateBlob(VertexSizeInBytes, &CPUVertexBufferPtr));
	memcpy(CPUVertexBufferPtr->GetBufferPointer(), InRenderingData->VertexData.data(), VertexSizeInBytes);

	ANALYSIS_HRESULT(D3DCreateBlob(IndexSizeInBytes, &CPUIndexBufferPtr));
	memcpy(CPUIndexBufferPtr->GetBufferPointer(), InRenderingData->IndexData.data(), IndexSizeInBytes);

	GPUVertexBufferPtr = ConstructDefaultBuffer(VertexBufferTempPtr, InRenderingData->VertexData.data(), VertexSizeInBytes);
	GPUIndexBufferPtr = ConstructDefaultBuffer(IndexBufferTempPtr, InRenderingData->IndexData.data(), IndexSizeInBytes);
	// ==================== END VertexBuffer IndexBuffer ====================





	// ==================== BEGIN PSO ====================
	D3D12_GRAPHICS_PIPELINE_STATE_DESC GPSDesc;
	//ZeroMemory(&GPSDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	memset(&GPSDesc, 0, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	// �󶨶�����ɫ�����벼�� ÿ����������������λ�á����ߡ����ߡ�UV1��UV2������ɫ
	GPSDesc.InputLayout.pInputElementDescs = InputElementDesc.data();
	GPSDesc.InputLayout.NumElements = (UINT)InputElementDesc.size();

	// �󶨸�ǩ��
	GPSDesc.pRootSignature = RootSignature.Get();

	// �󶨶�����ɫ��
	GPSDesc.VS.pShaderBytecode = reinterpret_cast<BYTE*>(VertexShader.GetBufferPointer());
	GPSDesc.VS.BytecodeLength = VertexShader.GetBufferSize();

	// ��������ɫ��
	GPSDesc.PS.pShaderBytecode = PixelShader.GetBufferPointer();
	GPSDesc.PS.BytecodeLength = PixelShader.GetBufferSize();

	// ���ù�դ��״̬
	GPSDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	GPSDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;// D3D12_FILL_MODE_SOLID D3D12_FILL_MODE_WIREFRAME

	GPSDesc.SampleMask = UINT_MAX;

	GPSDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	GPSDesc.NumRenderTargets = 1;

	GPSDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

	GPSDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);

	GPSDesc.SampleDesc.Count = GetEngine()->GetDXGISampleCount();
	GPSDesc.SampleDesc.Quality = GetEngine()->GetDXGISampleQuality();

	GPSDesc.RTVFormats[0] = GetEngine()->GetBackBufferFormat();

	GPSDesc.DSVFormat = GetEngine()->GetDepthStencilFormat();

	ANALYSIS_HRESULT(GetD3DDevice()->CreateGraphicsPipelineState(&GPSDesc, IID_PPV_ARGS(&PSO)));
	// ==================== END PSO ====================
}

void CMesh::PreDraw(float DeltaTime)
{
	GetGraphicsCommandList()->Reset(GetCommandAllocator().Get(), PSO.Get());
}

void CMesh::Draw(float DeltaTime)
{
	ID3D12DescriptorHeap* DescriptorHeaps[] = { CBVHeap.Get() };
	GetGraphicsCommandList()->SetDescriptorHeaps(_countof(DescriptorHeaps), DescriptorHeaps);

	GetGraphicsCommandList()->SetGraphicsRootSignature(RootSignature.Get());

	D3D12_VERTEX_BUFFER_VIEW VBV = GetVertexBufferView();
	GetGraphicsCommandList()->IASetVertexBuffers(0, 1, &VBV);// �󶨵���Ⱦ��ˮ���ϵ�slot

	D3D12_INDEX_BUFFER_VIEW IBV = GetIndexBufferView();
	GetGraphicsCommandList()->IASetIndexBuffer(&IBV);

	GetGraphicsCommandList()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	GetGraphicsCommandList()->SetGraphicsRootDescriptorTable(0, CBVHeap->GetGPUDescriptorHandleForHeapStart());

	GetGraphicsCommandList()->DrawIndexedInstanced(IndexSize, 1, 0, 0, 0);
}

void CMesh::PostDraw(float DeltaTime)
{
	XMFLOAT3 CameraPos = XMFLOAT3(0, 0, 5);

	// ==================== Begin View Matrix ====================
	XMVECTOR EyePosition = XMVectorSet(CameraPos.x, CameraPos.y, CameraPos.z, 1.0f);
	XMVECTOR FocusPosition = XMVectorZero();
	XMVECTOR UpDirection = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX ViewLookAt = XMMatrixLookAtLH(EyePosition, FocusPosition, UpDirection);
	XMStoreFloat4x4(&ViewMatrix, ViewLookAt);
	// ==================== End View Matrix ====================

	XMMATRIX Model = XMLoadFloat4x4(&WorldMatrix);
	XMMATRIX Projection = XMLoadFloat4x4(&ProjectionMatrix);
	XMMATRIX MVP = Model * ViewLookAt * Projection;

	FObjectTransformation ObjectTransformation;
	XMStoreFloat4x4(&ObjectTransformation.World, XMMatrixTranspose(MVP));

	ObjectConstants->Update(0, &ObjectTransformation);
}

CMesh* CMesh::CreateMesh(const FMeshRenderingData* InRenderingData)
{
	CMesh* NewMesh = new CMesh();
	NewMesh->BuildMesh(InRenderingData);
	return NewMesh;
}

D3D12_VERTEX_BUFFER_VIEW CMesh::GetVertexBufferView()
{
	D3D12_VERTEX_BUFFER_VIEW VBV;
	VBV.BufferLocation = GPUVertexBufferPtr->GetGPUVirtualAddress();
	VBV.SizeInBytes = VertexSizeInBytes;
	VBV.StrideInBytes = VertexStrideInBytes;
	return VBV;
}

D3D12_INDEX_BUFFER_VIEW CMesh::GetIndexBufferView()
{
	D3D12_INDEX_BUFFER_VIEW IBV;
	IBV.BufferLocation = GPUIndexBufferPtr->GetGPUVirtualAddress();
	IBV.SizeInBytes = IndexSizeInBytes;
	IBV.Format = IndexFormat;
	return IBV;
}

FObjectTransformation::FObjectTransformation()
	: World(FObjectTransformation::IdentityMatrix4x4())
{

}

XMFLOAT4X4 FObjectTransformation::IdentityMatrix4x4()
{
	return XMFLOAT4X4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
}
