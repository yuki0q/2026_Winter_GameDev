#include "../Application.h"
#include "VertexMaterial.h"

VertexMaterial::VertexMaterial(
	std::string shaderFileNameVS, int constBufFloat4SizeVS,
	std::string shaderFileNamePS, int constBufFloat4SizePS)
{

	// 頂点シェーダのロード
	shaderVS_ = LoadVertexShader(
		(Application::PATH_SHADER + shaderFileNameVS).c_str());

	// 定数バッファの確保サイズ(FLOAT4をいくつ作るか)
	constBufFloat4SizeVS_ = constBufFloat4SizeVS;

	// 頂点シェーダー用の定数バッファを作成
	constBufVS_ = CreateShaderConstantBuffer(sizeof(FLOAT4) * constBufFloat4SizeVS);

	// ピクセルシェーダのロード
	shaderPS_ = LoadPixelShader(
		(Application::PATH_SHADER + shaderFileNamePS).c_str());

	// 定数バッファの確保サイズ(FLOAT4をいくつ作るか)
	constBufFloat4SizePS_ = constBufFloat4SizePS;

	// ピクセルシェーダー用の定数バッファを作成
	constBufPS_ = CreateShaderConstantBuffer(sizeof(FLOAT4) * constBufFloat4SizePS);

	// テクスチャアドレス
	texAddress_ = TEXADDRESS::CLAMP;
}

void VertexMaterial::AddConstBufVS(const FLOAT4& contBufVS)
{

	if (constBufFloat4SizeVS_ > constBufsVS_.size())
	{
		constBufsVS_.emplace_back(contBufVS);
	}

}

void VertexMaterial::AddConstBufPS(const FLOAT4& contBufPS)
{

	if (constBufFloat4SizePS_ > constBufsPS_.size())
	{
		constBufsPS_.emplace_back(contBufPS);
	}

}

void VertexMaterial::SetConstBufVS(int idx, const FLOAT4& contBufVS)
{

	if (idx >= constBufsVS_.size())
	{
		return;
	}

	constBufsVS_[idx] = contBufVS;

}

void VertexMaterial::SetConstBufPS(int idx, const FLOAT4& contBufPS)
{

	if (idx >= constBufsPS_.size())
	{
		return;
	}

	constBufsPS_[idx] = contBufPS;

}

void VertexMaterial::SetTextureBuf(int slot, int texDiffuse)
{

	if (textures_.count(slot) == 0)
	{
		textures_.emplace(slot, texDiffuse);
	}
	else
	{
		textures_[slot] = texDiffuse;
	}

}

const std::map<int, int>& VertexMaterial::GetTextures(void) const
{
	return textures_;
}

VertexMaterial::TEXADDRESS VertexMaterial::GetTextureAddress(void) const
{
	return texAddress_;
}

void VertexMaterial::SetTextureAddress(TEXADDRESS texA)
{
	texAddress_ = texA;
}

int VertexMaterial::GetShaderVS(void) const
{
	return shaderVS_;
}

int VertexMaterial::GetShaderPS(void) const
{
	return shaderPS_;
}

const std::vector<FLOAT4>& VertexMaterial::GetConstBufsVS(void) const
{
	return constBufsVS_;
}

const std::vector<FLOAT4>& VertexMaterial::GetConstBufsPS(void) const
{
	return constBufsPS_;
}

int VertexMaterial::GetConstBufVS(void) const
{
	return constBufVS_;
}

int VertexMaterial::GetConstBufPS(void) const
{
	return constBufPS_;
}


VertexMaterial::~VertexMaterial(void)
{
	DeleteShader(shaderVS_);
	DeleteShaderConstantBuffer(constBufVS_);
	DeleteShader(shaderPS_);
	DeleteShaderConstantBuffer(constBufPS_);
}
