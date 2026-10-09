#pragma once
#include <string>
#include <vector>
#include <map>
#include <DxLib.h>

class VertexMaterial
{

public:

	// 設定できる最大のテクスチャの数
	static constexpr int MAX_TEXTURES = 4;

	// テクスチャアドレスタイプ
	enum class TEXADDRESS
	{
		NONE = 0,
		WRAP,
		MIRROR,
		CLAMP,
		BORDER,
		MAX
	};

	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="shaderFileName">シェーダーファイル名</param>
	/// <param name="constBufFloat4Size">定数バッファの確保サイズ(FLOAT4をいくつ作るか)</param>
	VertexMaterial(
		std::string shaderFileNameVS, int constBufFloat4SizeVS,
		std::string shaderFileNamePS, int constBufFloat4SizePS);

	// デストラクタ
	~VertexMaterial(void);

	// 定数バッファを追加
	void AddConstBufVS(const FLOAT4& contBufVS);
	void AddConstBufPS(const FLOAT4& contBufPS);

	// 定数バッファを更新
	void SetConstBufVS(int idx, const FLOAT4& contBufVS);
	void SetConstBufPS(int idx, const FLOAT4& contBufPS);

	// テクスチャを更新
	void SetTextureBuf(int slot, int texDiffuse);

	// シェーダ
	int GetShaderVS(void) const;
	int GetShaderPS(void) const;

	// 画像
	const std::map<int, int>& GetTextures(void) const;

	// テクスチャアドレスを取得
	TEXADDRESS GetTextureAddress(void) const;

	// テクスチャアドレスを設定
	void SetTextureAddress(TEXADDRESS texA);

	// 定数バッファハンドル
	int GetConstBufVS(void) const;
	int GetConstBufPS(void) const;

	// 定数バッファ
	const std::vector<FLOAT4>& GetConstBufsVS(void) const;
	const std::vector<FLOAT4>& GetConstBufsPS(void) const;

private:

	// シェーダ
	int shaderVS_;
	int shaderPS_;

	// 定数バッファの確保サイズ(FLOAT4をいくつ作るか)
	int constBufFloat4SizeVS_;
	int constBufFloat4SizePS_;

	// 定数バッファハンドル
	int constBufVS_;
	int constBufPS_;

	// テクスチャアドレス
	TEXADDRESS texAddress_;

	// 定数バッファ
	std::vector<FLOAT4> constBufsVS_;
	std::vector<FLOAT4> constBufsPS_;

	// 画像
	std::map<int, int> textures_;

};
