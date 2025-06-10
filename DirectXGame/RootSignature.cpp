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