#pragma once
#include <DxLib.h>
#include "../Common/Quaternion.h"
#include "../Object/Actor/ActorBase.h"
class Transform;
class ColliderSphere;
class SceneManager;

class Camera : public ActorBase
{

public:

	// カメラの初期座標
	static constexpr VECTOR DERFAULT_POS = { 0.0f, 200.0f, -500.0f };

	// カメラの初期角度
	static constexpr VECTOR DERFAULT_ANGLES = {
		0.0f, 0.0f, 0.0f
	};

	// カメラの回転量
	const float ROT_POW_DEG = 2.0f;
	const float ROT_POW_RAD = ROT_POW_DEG * DX_PI_F / 180.0f;

	// カメラの移動スピード
	static constexpr float SPEED = 20.0f;

	// カメラのクリップ範囲
	static constexpr float VIEW_NEAR = 10.0f;
	static constexpr float VIEW_FAR = 20000.0f;

	// 追従位置からカメラ位置までの相対座標
	//static constexpr VECTOR FOLLOW_CAMERA_LOCAL_POS = { 0.0f, 500.0f, -4000.0f };
	static constexpr VECTOR FOLLOW_CAMERA_LOCAL_POS = { 0.0f, 140.0f, -500.0f };

	// 追従位置から注視点までの相対座標
	//static constexpr VECTOR FOLLOW_TARGET_LOCAL_POS = { 0.0f, 20.0f, 5000.0f };
	static constexpr VECTOR FOLLOW_TARGET_LOCAL_POS = { 0.0f, 130.0f, 400.0f };

	// カメラのX回転上限度角
	static constexpr float LIMIT_X_UP_RAD = 40.0f * (DX_PI_F / 180.0f);
	static constexpr float LIMIT_X_DW_RAD = 10.0f * (DX_PI_F / 180.0f);

	// マウス感度
	static constexpr float MOUSE_SENSITIVITY = 0.0015f;

	// ズーム制限とスピードの定数を追加
	// 最も近づいたときの距離
	static constexpr float CAMERA_DIST_MIN = 150.0f;  
	// 最も離れたときの距離
	static constexpr float CAMERA_DIST_MAX = 800.0f;  
	static constexpr float ZOOM_SPEED = 25.0f;

	// カメラモード
	enum class MODE
	{
		NONE,
		FIXED_POINT,
		FREE,
		FOLLOW,
	};

	// 衝突判定種別
	enum class COLLIDER_TYPE
	{
		SPHERE,
		MAX,
	};

	// コンストラクタ
	Camera(void);

	// デストラクタ
	~Camera(void) override;

	// 初期化
	//void Init(void);

	// 更新
	void Update(void) override;

	// 描画前のカメラ設定
	void SetBeforeDraw(void);

	// デバッグ用描画
	void DrawDebug(void);

	// 解放
	void Release(void) override;

	// 座標の取得
	const VECTOR& GetPos(void) const;

	// 角度の取得
	const VECTOR& GetAngles(void) const;
	const Quaternion& GetQuaRot(void) const;

	// X回転を抜いたY軸のみのカメラ角度
	const Quaternion& GetQuaRotY(void) const;

	// 注視点の取得
	const VECTOR& GetTargetPos(void) const;

	// カメラの前方方向
	VECTOR GetForward(void) const;

	// カメラモードの変更
	void ChangeMode(MODE mode);

	// 追従対象の設定
	void SetFollow(const Transform* follow);

	// ズーム演出を開始させる関数
	void TriggerZoomIn(const VECTOR& zoomPoint1, const VECTOR& zoomPoint2, float duration);

	// イベントカメラの種類
	// 位置・向き・距離・時間は CameraShot.csv で調整する（無ければ既定値）
	enum class EVENT_SHOT
	{
		FRONT,	// プレイヤーの正面を映す（必殺技の準備など。EndFrontView を呼ぶまで続く）
		PARRY,	// パリィ成功：プレイヤーの横から寄って、受け止めた瞬間を映す
		SLASH,	// 斬撃を飛ばした：プレイヤーの横から、飛んでいく斬撃を映す
	};

	// イベントカメラが、元の向きへ戻る時の回り方（上から見た向き）
	// RIGHT は右回り（時計回り）：カメラが プレイヤーの 後ろ → 左 → 正面 → 右 の順に周る
	// LEFT  は左回り（反時計回り）：カメラが プレイヤーの 後ろ → 右 → 正面 → 左 の順に周る
	enum class RETURN_DIR
	{
		DEFAULT,	// CameraShot.csv の returnDir 列に従う
		SHORTEST,	// 戻り先までの近いほうの回り方
		RIGHT,		// 右回り
		LEFT,		// 左回り
	};

	// イベントカメラを開始する
	// ・開始前のカメラの向き・距離を覚えておき、演出が終わると滑らかに戻る
	// ・演出中は、プレイヤーの操作によるカメラの回転・ズームを受け付けない
	// ・演出の途中で別の演出が始まった場合は、新しい演出に切り替わる（戻り先は最初の向きのまま）
	// ・EndFrontView を呼ぶまで続く演出（FRONT）の間は、他の演出で上書きされない
	void StartEventShot(EVENT_SHOT shot, RETURN_DIR returnDir = RETURN_DIR::DEFAULT);

	// 必殺技の準備用：プレイヤーの正面を映す演出（FRONT）の開始・終了
	void StartFrontView(void);
	void EndFrontView(void);

	// イベントカメラの演出中か（映している間と、元の向きへ戻している間）
	bool IsEventShotActive(void) const { return eventMode_ != EVENT_MODE::NONE; }
	bool IsFrontViewActive(void) const { return IsEventShotActive(); }

protected:

	// リソースロード
	void InitLoad(void) override;

	// 大きさ、回転、座標の初期化
	void InitTransform(void) override;

	// 衝突判定の初期化
	void InitCollider(void) override;

	// アニメーションの初期化
	void InitAnimation(void) override;

	// 初期化後の個別処理
	void InitPost(void) override;

private:

	// 衝突時の押し戻し試行回数
	static constexpr int CNT_TRY_COLLISION_CAMERA = 30;

	// 衝突時の押し戻し量
	static constexpr float COLLISION_BACK_DIS = 2.0f;

	// 衝突判定用球体半径
	static constexpr float COL_CAPSULE_SPHERE = 50.0f;

	// カメラの補間移動率
	static constexpr float LERP_RATE_MOVE = 0.1f;

	// カメラの初期追従距離（例：400.0f）
	static constexpr float DEFAULT_FOLLOW_DIST = 400.0f;

	// カメラの更新前位置
	VECTOR prePos_;

	// カメラが追従対象とするTransform
	const Transform* followTransform_;

	// カメラモード
	MODE mode_;

	SceneManager& sceMng_;

	// カメラの位置
	//VECTOR pos_;

	// カメラ角度(rad)
	VECTOR angles_;

	// カメラ角度(Y軸のみ)
	Quaternion rotY_;

	// カメラ角度
	//Quaternion rot_;

	// 注視点
	VECTOR targetPos_;

	bool isCollision_;

	bool isZooming_ = false;          // ズーム演出中フラグ
	VECTOR zoomTargetPoint_ = { 0,0,0 }; // ズームするターゲット座標
	float zoomTimer_ = 0.0f;          // ズーム残り時間

	// 現在のカメラ追従距離（Z軸オフセット）
	float followDist_;

	// イベントカメラの段階
	enum class EVENT_MODE
	{
		NONE,		// 演出なし（通常のカメラ操作）
		SHOT,		// 演出のカメラへ寄せている（時間で終わる演出は、経過すると RETURNING へ）
		RETURNING,	// 演出前の向きへ戻している
	};
	EVENT_MODE eventMode_ = EVENT_MODE::NONE;

	// 1つの演出（ショット）の内容。カメラの向きは、開始時のプレイヤーの向きを基準にした相対値
	struct EventShot
	{
		float yaw;		// 横方向の回り込み量(rad)。0 でプレイヤーの真後ろ、π で真正面、正でプレイヤーの左側へ
		float pitch;	// 上下の角度(rad)。正で見下ろす
		float dist;		// プレイヤーからカメラまでの距離
		float hold;		// 映し続ける時間(秒)。負の値は EndFrontView を呼ぶまで続く
		float blendIn;	// 演出のカメラへ寄せる速さ（1フレームごとの補間率）
		float blendOut;	// 元の向きへ戻す速さ（同上）
		RETURN_DIR returnDir;	// 元の向きへ戻る時の回り方（DEFAULT 以外）
	};

	// 演出（ショット）の内容を取得（CameraShot.csv。無ければ既定値）
	static EventShot GetShot(EVENT_SHOT shot);

	// 今の演出と、その開始時のプレイヤーの向き(rad)・経過時間
	EventShot currentShot_ = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
	float shotBaseYaw_ = 0.0f;
	float shotTimer_ = 0.0f;

	// 戻る時の回り方（DEFAULT 以外に解決済みの値）と、戻る時のY角度の目標
	// （回り方を守るため、戻り始めに「何周分も含めた」目標の角度を決めて、そこへ向かう）
	RETURN_DIR returnDir_ = RETURN_DIR::SHORTEST;
	float returnTargetY_ = 0.0f;

	// 演出開始時のカメラ角度・追従距離（演出後に戻す先）
	VECTOR savedAngles_ = { 0.0f, 0.0f, 0.0f };
	float savedFollowDist_ = DEFAULT_FOLLOW_DIST;

	// 戻りきったと見なす誤差
	static constexpr float EVENT_END_EPSILON_RAD = 0.01f;
	static constexpr float EVENT_END_EPSILON_DIST = 1.0f;

	// 元の向きへ戻る段階に移る（回り方に従って、戻る時のY角度の目標を決める）
	void BeginReturn(void);

	// 演出中のカメラ更新
	void UpdateEvent(void);

	// マウスの移動量を捨てる（画面中央へ戻す）
	void DiscardMouseMove(void);

	// ズーム操作関数
	void ProcessZoom(void);

	// カメラの上方向
	//VECTOR cameraUp_;

	// カメラを初期位置に戻す
	void SetDefault(void);

	// 追従対象との位置同期を取る
	void SyncFollow(void);

	// カメラ操作
	void ProcessRot(bool isLimit);
	void ProcessMove(void);

	// カメラ回転(キーボード)
	void RotKeyboard(bool isLimit);

	// カメラ回転(ゲームパッド)
	void RotGamePad(bool isLimit);

	// カメラ回転(マウス) を追加
	void RotMouse(bool isLimit);

	// モード別更新ステップ
	void SetBeforeDrawFixedPoint(void);
	void SetBeforeDrawFree(void);
	void SetBeforeDrawFollow(void);

	// 衝突判定
	void Collision(void);
};
