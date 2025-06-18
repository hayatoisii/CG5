#include "IndexBuffer.h"
#include "KamataEngine.h"
#include "PipelineState.h"
#include "RootSignature.h"
#include "Shader.h"
#include "VertexBuffer.h"

#include <Windows.h>
#include <cassert>

using namespace KamataEngine;

void SetupPipelineState(PipelineState& pipelineState, RootSignature& rs, Shader& vs, Shader& ps) {

	// InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
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
	    {{-1.0f,-1.0f, 0.0f, 1.0f}, {0.0f,0.0f}}, // 左下
	    {{-1.0f, 3.0f, 0.0f, 1.0f}, {0.0f,2.0f}}, // 左上より上
		{{ 3.0f,-1.0f, 0.0f, 1.0f}, {2.0f,0.0f}}, // 右下より右
		{{ 3.0f,-1.0f, 0.0f, 1.0f}, {2.0f,0.0f}},
	};

	VertexBuffer vb;
	vb.Create(sizeof(vertices), sizeof(vertices[0]));

	VertexData* pGpuVertices = nullptr;
	vb.Get()->Map(0, nullptr, reinterpret_cast<void**>(&pGpuVertices));

	for (int i = 0; i < _countof(vertices); ++i) {
		pGpuVertices[i] = vertices[i];
	}

	uint16_t indices[] = {
	    0, 1, 2,
	};

	// IndexBuffer(IndexResource, IndexResourceView)の生成
	IndexBuffer ib;
	ib.Create(sizeof(indices), sizeof(indices[0]));

	uint16_t* pGpuIndices = nullptr;
	ib.Get()->Map(0, nullptr, reinterpret_cast<void**>(&pGpuIndices));

	for (int i = 0; i < _countof(indices); ++i) {
		pGpuIndices[i] = indices[i];
	}

	while (true) {
		if (KamataEngine::Update()) {
			break;
		}

		dxCommon->PreDraw();

		// コマンドを積む
		commandList->SetGraphicsRootSignature(rs.Get());     // RootSignatureの設定
		commandList->SetPipelineState(pipelineState.Get());  // PSOの設定する
		commandList->IASetVertexBuffers(0, 1, vb.GetView()); // VBVの設定
		commandList->IASetIndexBuffer(ib.GetView());         //
		// トポロジの設定
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		// 頂点数、インデックス数、インデックスの開始位置、インデックスのオフセット
		// commandList->DrawInstanced(3, 1, 0, 0);
		commandList->DrawIndexedInstanced(_countof(indices), 1, 0, 0, 0);

		dxCommon->PostDraw();
	}

	KamataEngine::Finalize();

	return 0;
}