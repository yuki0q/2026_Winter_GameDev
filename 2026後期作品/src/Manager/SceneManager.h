#pragma once
#include <chrono>
#include <DxLib.h>
class SceneBase;
class Fader;
class Camera;

class SceneManager
{

public:

	// 背景色
	static constexpr int BACKGROUND_COLOR_R = 0;
	static constexpr int BACKGROUND_COLOR_G = 139;
	static constexpr int BACKGROUND_COLOR_B = 139;

	// ディレクショナルライトの方向
	static constexpr VECTOR LIGHT_DIRECTION = { 0.3f, -0.7f, 0.8f };

	// シーン管理用
	enum class SCENE_ID
	{
		NONE,
		TITLE,
		GAME,
		PAUSE,
		GAMEOVER,
		CLEAR,
		DEBUG,
	};

	// インスタンスの生成
	static void CreateInstance(void);

	// インスタンスの取得
	static SceneManager& GetInstance(void);

	// 初期化
	void Init(void);

	// 3Dの初期化
	void Init3D(void);

	// 更新
	void Update(void);

	// 描画
	void Draw(void);

	// リソースの破棄
	void Destroy(void);

	// 状態遷移
	void ChangeScene(SCENE_ID nextId);

	// シーンIDの取得
	SCENE_ID GetSceneID(void);

	// デルタタイムの取得
	float GetDeltaTime(void) const;

	// カメラの取得
	Camera* GetCamera(void) const;

	// カメラシェイクの強さを設定
	void SetShakePower(float power);

private:

	// 静的インスタンス
	static SceneManager* instance_;

	SCENE_ID sceneId_;
	SCENE_ID waitSceneId_;

	// フェード
	Fader* fader_;

	// 各種シーン
	SceneBase* scene_;
	// ポーズ画面用のシーン保持変数
	SceneBase* pauseScene_;  

	// カメラ
	Camera* camera_;

	// メインスクリーン
	int mainScreen_;

	// シーン遷移中判定
	bool isSceneChanging_;

	// シェイク用
	int shakePs_;          // ピクセルシェーダー
	int shakeCbuf_;        // 定数バッファ
	float shakePower_;     // 現在の強さ(UV単位)

	// ロード画面の進行段階
	enum class LOAD_PHASE
	{
		NONE,		// ロード画面ではない
		PREPARE,	// ロード画面を1フレーム描画するための待ち
		WARMUP,		// 新シーン生成後の準備更新・最低表示時間の待ち
	};

	LOAD_PHASE loadPhase_;
	float loadTimer_;		// 新シーン生成後の経過時間
	float loadAnimTimer_;	// ロード画面のアニメーション用
	int warmupCount_;		// 準備更新を行った回数
	bool isWarmingUp_;		// 準備更新中（この間の遷移要求は無視する）
	int loadingFont_;		// ロード画面用フォント

	// デルタタイム
	std::chrono::system_clock::time_point preTime_;
	float deltaTime_;

	// デフォルトコンストラクタをprivateにして、
	// 外部から生成できない様にする
	SceneManager(void);

	// コピーコンストラクタも同様
	SceneManager(const SceneManager& instance) = default;

	// デストラクタも同様
	~SceneManager(void) = default;

	// デルタタイムをリセットする
	void ResetDeltaTime(void);

	// シーン遷移
	void DoChangeScene(SCENE_ID sceneId);

	// フェード
	void Fade(void);

	// ロード画面の更新・描画
	void UpdateLoading(void);
	void DrawLoading(void);
};