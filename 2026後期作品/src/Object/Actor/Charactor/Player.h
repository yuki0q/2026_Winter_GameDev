#pragma once
#include "CharactorBase.h"
#include "PlayerDataTable.h"
#include "../Weapon/WeaponBase.h"
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
		WEAPON_RUN,			// 片手剣ダッシュ
		FAST_RUN,
		JUMP,
		ATTACK_1,	// 片手剣攻撃１
		ATTACK_2,	// 片手剣攻撃２
		ATTACK_3,	// 片手剣攻撃３
		ATTACK_4,	// 片手剣攻撃４
		DASHATTACK,	// 片手剣ダッシュ攻撃
		AVOID,			// 回避
		PARRY,		// 片手剣パリィ
		PARRYED,		// 片手剣パリィ後
		PARRYATTACK,	// 片手剣パリィ攻撃
		SHEATHING,	// 片手剣抜刀
		UNSHEATHED,	// 片手剣納刀
		DAMAGE,
		DIE
		//MAX,
	};

	// プレイヤーの状態（ステート）
	enum class STATE
	{
		IDLE,
		WALK,
		RUN,
		AVOID,
		ATTACK,
		PARRY,
		PARRYED,
		PARRYATTACK,
		JUMP,
		DAMAGE,
		DIE,
		END,
	};

	Player(void);
	virtual ~Player(void);

	void Draw(void);
	void Release(void);

	void OnDamage(int damage, VECTOR attackedPos);
	void InDamage(int damage);
	void AddHP(int addDamage);

	void SetPosition(const VECTOR& pos);

	// 現在パリィ中（パリィステート）かどうかを返す
	bool IsParry(void) const { return currentState_ == STATE::PARRY; }

	void ClearParry(void);

	// 指定した座標のほうを向く関数
	void LookAt(const VECTOR& targetPos);

	// 敵の攻撃予備動作に対する警告を受けているか設定・取得
	void SetWarning(bool isWarning) { isWarning_ = isWarning; }
	bool IsWarning(void) const { return isWarning_; }

	int GetAttackDamage(void) const;

private:

	static constexpr VECTOR PLAYER_DEFAULT_ROT_LOCAL = { 0.0f,180.0f * DX_PI_F / 180.0f,0.0f };

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

	// 武器を生成して、weapon_param.csv の値でスロットに装備する
	void EquipFromTable(WEAPON_TYPE type, int modelHandle);

	// 更新系
	void UpdateProcess(void) override;
	void UpdateProcessPost(void) override;

	// 各ステートの更新処理
	void UpdateIdle(void);
	void UpdateWalk(void);
	void UpdateRun(void);
	void UpdateAvoid(void);
	void UpdateAttack(void);
	void UpdateParry(void);
	void UpdateParryed(void);
	void UpdateParryAttack(void);
	void UpdateDamage(void);
	void UpdateDie(void);
	void UpdateEnd(void);

	void DelayRotate(void) override;

	// 移動入力と移動処理を行う共通関数
	VECTOR GetInputDirection(bool& outIsDash);
	void MoveDirection(const VECTOR& dir, float speed);

	// ステート遷移関数
	void ChangeState(STATE newState);

	// ダッシュ入力のリセット用
	bool isDashEnable_;

	// パリィ後の後退用スピード
	float parryedBackSpeed_;

	void DrawDegug(void);

	// 現在のステート
	STATE currentState_; 

	// コンボ用変数
	// 現在の攻撃段階
	int attackStep_; 
	// 攻撃中の先行入力フラグ
	bool hasComboInput_;

	// コンボ猶予（攻撃が途切れても、一定時間は続きの段から再開できる）
	// 猶予の残り時間（秒）。0 なら猶予なし
	float comboTimer_;
	// 猶予中に再開する時の基準になる、最後に出し切った攻撃段
	int lastAttackStep_;

	// 猶予時間を減らす（毎フレーム）
	void UpdateComboTimer(void);
	// 通常攻撃を始める時の段（猶予中なら続きの段、そうでなければ1段目）
	int GetStartAttackStep(void) const;
	// コンボと猶予を完全にリセットする
	void ResetCombo(void);

	// CSV(player_param.csv)から読み込んだ共通パラメータ
	PlayerParam param_;

	// メイン武器の種類に対応する動きのパラメータ(player_style.csv)
	PlayerStyle style_;

	// 右手(メイン)・左手(サブ)の武器種別とモデルハンドル
	WEAPON_TYPE mainWeaponType_;
	WEAPON_TYPE subWeaponType_;
	int mainHandle_;
	int subHandle_;
	
	bool isParry_; 
	// 警告フラグ
	bool isWarning_;
};

