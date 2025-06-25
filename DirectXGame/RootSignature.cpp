#include "RootSignature.h"
#include "KamataEngine.h"

using namespace KamataEngine;

// RootSignatureを生成する
void RootSignature::Create() {
	 // すでにインスタンスがあるなら解放する
	if (rootSignature_) {
		rootSignature_->Release();
		rootSignature_ = nullptr;
	}

	// クラス内で取得するために追加
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// RootSignatureの生成
	// 構造体的にデータを用意する
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// でスクリプタレンジ
	D3D12_DESCRIPTOR_RANGE srvDescRange[1]{};
	// t0 レジスタを再利用可能にする
	srvDescRange[0].BaseShaderRegister = 0;                      // 0から始まる
	srvDescRange[0].NumDescriptors = 1;                          // 数は1つ
	srvDescRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // SRV
	srvDescRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; //

	// RootParameterの用意
	// 複数設定できるので配列の構造をしている。今回は１つだけなので、長さ1の配列として用意する
	D3D12_ROOT_PARAMETER rootParameters[1]{};

	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;  //
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;            //
	rootParameters[0].DescriptorTable.pDescriptorRanges = srvDescRange;            //
	rootParameters[0].DescriptorTable.NumDescriptorRanges = _countof(srvDescRange);//

	descriptionRootSignature.pParameters = rootParameters;             //
	descriptionRootSignature.NumParameters = _countof(rootParameters); //

	// Samplerの設定
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;     //
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;   //
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;   //
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;   //
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; //
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;                   //
	staticSamplers[0].ShaderRegister = 0;                           //
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; //

	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);


	ID3D10Blob* signatureBlob = nullptr;
	ID3D10Blob* errorBlog = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlog);
	if (FAILED(hr)) {
		DebugText::GetInstance()->ConsolePrintf(reinterpret_cast<char*>(errorBlog->GetBufferPointer()));
		assert(false);
	}

	// バイナリをもとに生成
	ID3D12RootSignature* rootSignature = nullptr;
	hr = dxCommon->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), 
		signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));

	// signatureBlobはRootSignatureの生成後解放してもいい
	signatureBlob->Release();

	// 生成したRootSignatureをとっておく
	rootSignature_ = rootSignature;
}

// 生成したRootSignatureを返す
ID3D12RootSignature* RootSignature::Get() { 
	return rootSignature_;
}

// コンストラクタ
RootSignature::RootSignature() {
}

// デストラクタ
RootSignature::~RootSignature() {
	if (rootSignature_) {
		rootSignature_->Release();
		rootSignature_ = nullptr;
	}
}