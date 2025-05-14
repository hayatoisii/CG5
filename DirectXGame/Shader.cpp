#include "Shader.h"
#include <d3dcompiler.h>
#include <cassert>

void Shader::Load(const std::wstring& filePath, const std::string& shaderModel) {

	ID3D10Blob* shaderBlob = nullptr;
	ID3D10Blob* errorBlob = nullptr;

	HRESULT hr = D3DCompileFromFile(
	    filePath.c_str(), // シェーダファイル名
	    nullptr,
	    D3D_COMPILE_STANDARD_FILE_INCLUDE,               // インクルード可能にする
	    "main", shaderModel.c_str(),                     // エントリーポイント名、シェーダーモデル指定
	    D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, // デバッグ用設定
	    0, &shaderBlob, &errorBlob);

	if (FAILED(hr)) {
		if (errorBlob) {
			OutputDebugStringA(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		}
		assert(false);
	}
	blob_ = shaderBlob;
}

ID3DBlob* Shader::GetBlob() { return blob_; }

Shader::Shader() {}

Shader::~Shader() {

	if (blob_) {
		blob_->Release();
		blob_ = nullptr;
	}

}
