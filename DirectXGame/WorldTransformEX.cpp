#include "WorldTransformEX.h"

using namespace KamataEngine;
using namespace KamataEngine::MathUtility;

// Scale, Rotation, Translate 行列から World行列を計算、
// そして定数バッファへの転送を行う
void WorldTransformEX::UpdateMatrix() {
	// World変換行列を計算し、matWorld_ に格納する
	matWorld_ = MakeAffineMatrix();
	// 定数バッファへ転送する
	TransferMatrix();
}

Matrix4x4 WorldTransformEX::MakeAffineMatrix() { 
	// Scale Matrix
	Matrix4x4 matScale = MakeScaleMatrix(scale_);

	// Rotation Matrix
	Matrix4x4 matRotX = MakeRotateXMatrix(rotation_.x);
	Matrix4x4 matRotY = MakeRotateYMatrix(rotation_.y);
	Matrix4x4 matRotZ = MakeRotateZMatrix(rotation_.z);
	Matrix4x4 matRot = matRotX * matRotY * matRotZ;

	// Translation Matrix
	Matrix4x4 matTrans = MakeTranslateMatrix(translation_);

	// World Matrix
	Matrix4x4 matWorld = matScale * matRot * matTrans;

	return matWorld;
}
