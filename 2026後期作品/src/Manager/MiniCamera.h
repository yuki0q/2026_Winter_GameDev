#pragma once
#include <DxLib.h>
#include "../Common/Vector2.h"
class Camera;

class MiniCamera
{

public:

	// ミニ画面のサイズ
	static constexpr int SIZE_X = 200;
	static constexpr int SIZE_Y = 200;

	// 背景色
	static constexpr int BACKGROUND_COLOR_R = 200;
	static constexpr int BACKGROUND_COLOR_G = 200;
	static constexpr int BACKGROUND_COLOR_B = 200;

	// ミニカメラモデルの大きさ
	static constexpr float SCALE = 3.0f;

	// コンストラクタ
	MiniCamera(Camera* camera);

	// デストラクタ
	~MiniCamera(void);

	// 初期化
	void Init(void);

	// 更新
	void Update(void);

	// ミニカメラ用視野内の描画
	void DrawScreen(void);

	// 描画
	void Draw(void);

	// 解放
	void Release(void);

	// スクリーンのハンドルID取得
	int GetScreen(void) const;

private:

	// カメラ
	Camera* camera_;

	// スクリーンのハンドルID
	int miniScreen_;

	// ミニカメラモデルのハンドルID
	int modelId_;
	
	// ミニカメラ用の描画領域サイズ
	Vector2 size_;

	// ミニカメラモデルの位置
	VECTOR pos_;

};
