#pragma once
#include "CharactorBase.h"
#include <vector>

class Player : public CharactorBase
{

public:

	// アニメーション
	enum class ANIM_TYPE
	{
		IDLE,
		WALK,
		RUN,
		SS_RUN,			// 片手剣ダッシュ
		FAST_RUN,
		JUMP,
		SS_ATTACK_1,	// 片手剣攻撃１
		SS_ATTACK_2,	// 片手剣攻撃２
		SS_ATTACK_3,	// 片手剣攻撃３
		SS_ATTACK_4,	// 片手剣攻撃４
		SS_DASHATTACK,	// 片手剣ダッシュ攻撃
		AVOID,
		SS_PARRY,		// 片手剣パリィ
		SS_SHEATHING,	// 片手剣抜刀
		SS_UNSHEATHED	// 片手剣納刀
		//MAX,
	};

	// プレイヤーの状態（ステート）
	enum class STATE
	{
		IDLE,
		WALK,
		RUN,
		ATTACK,
		PARRY,
		JUMP,
	};

	Player(void);
	virtual ~Player(void);

	void Update(void);
	void Draw(void);
	void Release(void);


private:

	// 衝突判定用線分開始
	static constexpr VECTOR COL_LINE_START_LOCAL_POS = { 0.0f, 120.0f, 0.0f };
	// 衝突判定用線分終了
	static constexpr VECTOR COL_LINE_END_LOCAL_POS = { 0.0f, 0.0f, 0.0f };
	// 衝突判定用カプセル上部球体
	static constexpr VECTOR COL_CAPSULE_TOP_LOCAL_POS = { 0.0f, 145.0f, 0.0f };
	// 衝突判定用カプセル下部球体
	static constexpr VECTOR COL_CAPSULE_DOWN_LOCAL_POS = { 0.0f, 40.0f, 0.0f };
	// 衝突判定用カプセル球体半径
	static constexpr float COL_CAPSULE_RADIUS = 35.0f;

	static constexpr VECTOR PLAYER_DEFAULT_SCALE = { 1.0f,1.0f,1.0f };
	static constexpr VECTOR PLAYER_DEFAULT_ROT_LOCAL = { 0.0f,180.0f * DX_PI_F / 180.0f,0.0f };
	static constexpr VECTOR DEFAULT_POS = { 0.0f,300.0f,0.0f };
	static constexpr VECTOR DEFAULT_LOCAL_POS = { 100.0f,0.0f,100.0f };

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

	// 更新系
	void UpdateProcess(void) override;
	void UpdateProcessPost(void) override;

	// 各ステートの更新処理
	void UpdateIdle(void);
	void UpdateWalk(void);
	void UpdateRun(void);
	void UpdateAttack(void);
	void UpdateParry(void);

	// 移動入力と移動処理を行う共通関数
	VECTOR GetInputDirection(bool& outIsDash);
	void MoveDirection(const VECTOR& dir, float speed);

	// ステート遷移関数
	void ChangeState(STATE newState);

	// ダッシュ入力のリセット用
	bool isDashEnable_;

	void DrawDegug(void);

	// 現在のステート
	STATE currentState_; 

	// コンボ用変数
	// 現在の攻撃段階（1?3）
	int attackStep_; 
	// 攻撃中の先行入力フラグ
	bool hasComboInput_;

	int swordHandle_;

	int shieldHandle_;
	
	bool isParry_;
	bool isAttack1_;
};

