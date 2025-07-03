#include "IndexBuffer.h"
#include "KamataEngine.h"
#include "PipelineState.h"
#include "RootSignature.h"
#include "Shader.h"
#include "VertexBuffer.h"
#include "WorldTransformEX.h"

#include <Windows.h>
#include <cassert>

using namespace KamataEngine;

// PipelineStateObjectの生成
void SetupPipelineState(PipelineState& pipelineState, RootSignature& rs, Shader& vs, Shader& ps);

// RenderTextureResourceの生成
ID3D12Resource* CreateRenderTextureResource(ID3D12Device* device, uint32_t width, uint32_t height, DXGI_FORMAT format, const FLOAT* clearColor);

// DepthStencilTextureResourceの生成
ID3D12Resource* CreateDepthStencilTextureResource(ID3D12Device* device, uint32_t width, uint32_t height);

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	KamataEngine::Initialize(L"LE3C_01_イシイ_ハヤト");

	// DirectXCommonインスタンスの取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// DirectXCommonクラスが管理している、ウィンドウの幅と高さの値の取得
	int32_t w = dxCommon->GetBackBufferWidth();
	int32_t h = dxCommon->GetBackBufferHeight();
	DebugText::GetInstance()->ConsolePrintf(std::format("width: {}, height: {}\n", w, h).c_str());

	// DirectXCommonクラスが管理している、コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = dxCommon->GetCommandList();

	// RootSignatureの生成
	RootSignature rs;
	rs.Create();

	Shader vs;
	vs.LoadDxc(L"Resources/shaders/TestVS.hlsl", L"vs_6_0");
	assert(vs.GetDxcBlob() != nullptr);

	Shader ps;
	ps.LoadDxc(L"Resources/shaders/TestPS.hlsl", L"ps_6_0");
	assert(ps.GetDxcBlob() != nullptr);

	// PipelineStateの生成
	PipelineState pipelineState;
	SetupPipelineState(pipelineState, rs, vs, ps);

	// リソース確保含め、
	struct VertexData {
		Vector4 position;
		Vector2 texcoord;
	};

	/*/
	// 頂点データの準備
	VertexData vertices[] = {
	    {0.0f,  0.5f,  0.0f, 1.0f}, // 左下
	    {0.5f,  -0.5f, 0.0f, 1.0f}, // 上
	    {-0.5f, -0.5f, 0.0f, 1.0f}, // 右下
	};
	/*/

	// 真っ白
	VertexData vertices[] = {
	    {{-1.0f, 1.0f, 0.0f, 1.0f},  {0.0f, 0.0f}}, // 左下
	    {{1.0f, 1.0f, 0.0f, 1.0f},   {1.0f, 0.0f}}, // 左上より上
	    {{-1.0f, -1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // 右下より右
	    {{1.0f, -1.0f, 0.0f, 1.0f},  {1.0f, 1.0f}},
	};

	uint16_t indices[] = {0, 1, 2, 2, 1, 3};

	VertexBuffer vb;
	vb.Create(sizeof(vertices), sizeof(vertices[0]));

	VertexData* pGpuVertices = nullptr;
	vb.Get()->Map(0, nullptr, reinterpret_cast<void**>(&pGpuVertices));

	for (int i = 0; i < _countof(vertices); ++i) {
		pGpuVertices[i] = vertices[i];
	}

	// IndexBuffer(IndexResource, IndexResourceView)の生成
	IndexBuffer ib;
	ib.Create(sizeof(indices), sizeof(indices[0]));

	uint16_t* pGpuIndices = nullptr;
	ib.Get()->Map(0, nullptr, reinterpret_cast<void**>(&pGpuIndices));

	for (int i = 0; i < _countof(indices); ++i) {
		pGpuIndices[i] = indices[i];
	}

	// Resource生成、Heap生成、View生成で再利用される変数の準備
	ID3D12Device* device = dxCommon->GetDevice();
	HRESULT hr;

	// RenderTextureResourceの生成
	// 画面クリア色
	const FLOAT kRenderTargetClearColor[4] = {1.0f, 0.0f, 0.0f, 1.0f};

	ID3D12Resource* renderTextureResource = CreateRenderTextureResource(device, WinApp::kWindowWidth, WinApp::kWindowHeight, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, kRenderTargetClearColor);

	// 1.RTV用のDescriptorHeapを生成する
	ID3D12DescriptorHeap* rtvDescriptorHeap = nullptr;

	D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc = {};
	rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // RTV
	rtvDescriptorHeapDesc.NumDescriptors = 1;                    // Descriptorの戸数は1

	hr = device->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap));
	assert(SUCCEEDED(hr));

	// CPU側から見たHANDLEを取得しておく
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandleCPU = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	// 2.RTV用のViewの生成
	device->CreateRenderTargetView(
	    renderTextureResource, // Viewと関連付けたいリソース
	    nullptr,               // RTVの詳細情報(Desc:Description,構成内容のく述)

	    rtvHandleCPU // RTV用のディスクリプタヒープの CPU Handle
	);

	// 0.DepthStencilTextureResourceの生成
	ID3D12Resource* depthStencilResource = CreateDepthStencilTextureResource(device, WinApp::kWindowWidth, WinApp::kWindowHeight);

	// 1.DSV用のDescriptorHeapの作成
	ID3D12DescriptorHeap* dsvDescriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC dsvDescriptorHeapDesc = {};
	dsvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;   //
	dsvDescriptorHeapDesc.NumDescriptors = 1;                      //
	dsvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE; //

	hr = device->CreateDescriptorHeap(&dsvDescriptorHeapDesc, IID_PPV_ARGS(&dsvDescriptorHeap));
	assert(SUCCEEDED(hr));

	// CPU側から見たDSVのHANDLEを取得しておく
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandleCPU = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	// 2.DSV用のViewの生成
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;                //
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D; //

	// DSVHeapの先頭にDSVを作る
	device->CreateDepthStencilView(depthStencilResource, &dsvDesc, dsvHandleCPU);

	// 1. SRV用のDescriptorHeapの生成
	ID3D12DescriptorHeap* srvDescriptorHeap = nullptr;

	D3D12_DESCRIPTOR_HEAP_DESC srvDescriptorHeapDesc = {};
	srvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;     // SRV
	srvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; //
	srvDescriptorHeapDesc.NumDescriptors = 1;                                //

	hr = device->CreateDescriptorHeap(&srvDescriptorHeapDesc, IID_PPV_ARGS(&srvDescriptorHeap));
	assert(SUCCEEDED(hr));

	// CPU側から見たSRVのHANDLE、GPU側からみたHANDLEを取得しておく
	D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU = srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU = srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

	// 2. SRV(Shader  Resource View)もの作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;                           //
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING; //
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;                      //
	srvDesc.Texture2D.MipLevels = 1;                                            //

	device->CreateShaderResourceView(
	    renderTextureResource, // Viewと関連付けたいリソース
	    &srvDesc,              // SRVの詳細情報(Desc:Description、構成内容の記述)
	    srvHandleCPU           // SRV用のディスクリプタヒープの CPU Handle
	);

	// アプリで利用する3Dもでる
	// 被写体の準備
	Model* model = Model::CreateFromOBJ("terrain");

	WorldTransformEX worldTransform;
	worldTransform.Initialize();
	worldTransform.scale_ = Vector3(1.0f, 1.0f, 1.0f);

	// カメラの準備
	Camera camera;
	camera.Initialize();
	camera.translation_ = Vector3(0.0f, 1.0f, 0.0f);


	while (true) {
		if (KamataEngine::Update()) {
			break;
		}

		// world変換行列の定数バッファへの転送
		worldTransform.rotation_.y += 0.005f;
		worldTransform.UpdateMatrix();

		// cameraの更新と定数バッファへの転送
		camera.UpdateMatrix();

        D3D12_RESOURCE_BARRIER barrierRTV{};
		barrierRTV.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrierRTV.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrierRTV.Transition.pResource = renderTextureResource;
		barrierRTV.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		barrierRTV.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		commandList->ResourceBarrier(1, &barrierRTV);
		// 描画先のRTVとDSVを設定
		commandList->OMSetRenderTargets(1, &rtvHandleCPU, false, &dsvHandleCPU);

		// Viewportの設定
		D3D12_VIEWPORT viewport{};
		viewport.Width = WinApp::kWindowWidth;
		viewport.Height = WinApp::kWindowHeight;
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		commandList->RSSetViewports(1, &viewport);

		// ScissorRectの設定
		D3D12_RECT scissorRect{};
		// 基本的にビューポートと同じ矩形が構成されるようにする
		scissorRect.left = 0;
		scissorRect.right = WinApp::kWindowWidth;
		scissorRect.top = 0;
		scissorRect.bottom = WinApp::kWindowHeight;
		commandList->RSSetScissorRects(1, &scissorRect);

		// 全画面clear
		commandList->ClearRenderTargetView(rtvHandleCPU, kRenderTargetClearColor, 0, nullptr);
		// 指定した深度で画面全体をクリアする
		commandList->ClearDepthStencilView(dsvHandleCPU, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		Model::PreDraw(commandList);
		model->Draw(worldTransform, camera);
		Model::PostDraw();

		// --- 描画後バリア（RTV→SRV） ---
		D3D12_RESOURCE_BARRIER barrierSRV{};
		barrierSRV.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrierSRV.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrierSRV.Transition.pResource = renderTextureResource;
		barrierSRV.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrierSRV.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		commandList->ResourceBarrier(1, &barrierSRV);

		dxCommon->PreDraw();

		// コマンドを積む
		commandList->SetGraphicsRootSignature(rs.Get());    // RootSignatureの設定
		commandList->SetPipelineState(pipelineState.Get()); // PSOの設定する

		commandList->IASetVertexBuffers(0, 1, vb.GetView()); // VBVの設定
		commandList->IASetIndexBuffer(ib.GetView());
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// 使用するディスクリプタヒープの設定
		commandList->SetDescriptorHeaps(srvDescriptorHeap->GetDesc().NumDescriptors, &srvDescriptorHeap);

		// SRVのDescripterTableの先頭を設定
		commandList->SetGraphicsRootDescriptorTable(0, srvHandleGPU);

		// 画面を覆うポリゴンの描画 
		commandList->DrawIndexedInstanced(_countof(indices), 1, 0, 0, 0);

		dxCommon->PostDraw();
	}

	// 解放　
	delete model;

	renderTextureResource->Release();
	srvDescriptorHeap->Release();
	rtvDescriptorHeap->Release();

	depthStencilResource->Release();
	dsvDescriptorHeap->Release();

	KamataEngine::Finalize();

	return 0;
}

void SetupPipelineState(PipelineState& pipelineState, RootSignature& rs, Shader& vs, Shader& ps) {

	// InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	// BlendState 今回は不透明
	D3D12_BLEND_DESC blendDesc = {};
	// すべての色要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	// RasterizerState
	D3D12_RASTERIZER_DESC rasterizerDesc = {};
	// 裏面(反時計回り)をカリングする
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	// 塗りつぶしモードをソリッドにする(ワイヤーフレームなら　D3D12_FILL_MOOE_WIREFRAME)
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	// PSO(PipelineStateObject)の生成
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc = {};
	graphicsPipelineStateDesc.pRootSignature = rs.Get();                                                    // RootSignature
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;                                                // InputLayout
	graphicsPipelineStateDesc.VS = {vs.GetDxcBlob()->GetBufferPointer(), vs.GetDxcBlob()->GetBufferSize()}; // vertexShader
	graphicsPipelineStateDesc.PS = {ps.GetDxcBlob()->GetBufferPointer(), ps.GetDxcBlob()->GetBufferSize()}; // pixelShader
	graphicsPipelineStateDesc.BlendState = blendDesc;                                                       // BlendState
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;                                             // RasterizerState

	// 書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1; // 1つのRTVに書き込む
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 利用するトポロジ(形状)のタイプ。三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// どのように画面に色を打ち込むかの設定
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	pipelineState.Create(graphicsPipelineStateDesc);
}

// RenderTextureResourceの生成
ID3D12Resource* CreateRenderTextureResource(ID3D12Device* device, uint32_t width, uint32_t height, DXGI_FORMAT clearFormat, const FLOAT* clearColor) {

	// 1.生成するRenderTextureの　Descの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(width);                             // RenderTextureの幅
	resourceDesc.Height = UINT(height);                           // Textureの高さ
	resourceDesc.MipLevels = 1;                                   // mipmapの数
	resourceDesc.DepthOrArraySize = 1;                            // 奥行　or　配列Textureの配列数
	resourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;        // TextureのFormat
	resourceDesc.SampleDesc.Count = 1;                            // サンプリングカウント　1固定
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;  // Textureの時限数。普段使ってるのは2次元
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET; // RenderTargetとして使う通知

	// 2.利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // VRAM上に作る

	// 3.ClaarValueの用意
	D3D12_CLEAR_VALUE clearValue;
	clearValue.Format = clearFormat;
	clearValue.Color[0] = clearColor[0];
	clearValue.Color[1] = clearColor[1];
	clearValue.Color[2] = clearColor[2];
	clearValue.Color[3] = clearColor[3];

	// 4.RenderTextureResoureceの生成
	ID3D12Resource* resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
	    &heapProperties,                            // Heapの設定
	    D3D12_HEAP_FLAG_NONE,                       // Heapの特殊な設定
	    &resourceDesc,                              // Resourceの設定
	    D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, // 初期状態
	    &clearValue,                                // クリア値
	    IID_PPV_ARGS(&resource)                     // 生成したリソースのポインタを受け取る
	);
	assert(SUCCEEDED(hr));

	return resource;
}

ID3D12Resource* CreateDepthStencilTextureResource(ID3D12Device* device, uint32_t width, uint32_t height) {
	// 1.生成するDepthStencilTextureのDescの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width;
	resourceDesc.Height = height;
	resourceDesc.MipLevels = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Format = DXGI_FORMAT_D32_FLOAT;

	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	// 2.利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	// 深度値のクリア設定
	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;

	// 3.Resourceの生成
	ID3D12Resource* resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
	    &heapProperties,                  // Heapの設定
	    D3D12_HEAP_FLAG_NONE,             // Heapの特殊な設定
	    &resourceDesc,                    // Resourceの設定
	    D3D12_RESOURCE_STATE_DEPTH_WRITE, // 深度値を書き込み状態にしておく
	    &depthClearValue,                 // clear最適値
	    IID_PPV_ARGS(&resource)           // 作成するResourceポインタへのポインタ
	);
	assert(SUCCEEDED(hr));

	return resource;
}