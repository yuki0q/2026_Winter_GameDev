#pragma once
#include <string>
#include <vector>

class Resource
{

public:

	// リソースタイプ
	enum class TYPE
	{
		NONE,
		IMG,
		IMGS,
		MODEL,
		EFFEKSEER,
		FONT
	};

	// コンストラクタ
	Resource(void);
	// コンストラクタ
	Resource(TYPE type, const std::string& path);
	// コンストラクタ(IMGS用)
	Resource(TYPE type, const std::string& path, int numX, int numY, int sizeX, int sizeY);
	// コンストラクタ(Font用)
	Resource(TYPE type, const std::string& fontFilePath, const std::string& fontName, int size, int thickness, int fontType);

	// デストラクタ
	~Resource(void);

	// 読み込み
	void Load(void);

	// 解放
	void Release(void);

	// 複数画像ハンドルを別配列にコピー
	void CopyHandle(int* imgs) const;

	// リソースタイプ
	TYPE type_;

	// リソースの読み込み先
	std::string path_;

	// 画像とモデルのハンドルID
	int handleId_;

	// IMGS::LoadDivGraph用
	int* handleIds_;
	int numX_;
	int numY_;
	int sizeX_;
	int sizeY_;

	// フォント用メンバ変数
	std::string fontName_;  // フォント名（例: "スマートフォントUI"）
	int fontSize_;          // フォントサイズ
	int fontThickness_;     // フォントの太さ
	int fontType_;          // 描画タイプ（アンチエイリアスなど）
	bool isFontRegistered_; // AddFontResourceExを実行したかどうかのフラグ

	// モデル複製用
	std::vector<int> duplicateModelIds_;

};
