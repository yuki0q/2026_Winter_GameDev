#pragma once
#include "EnemyBase.h"

class EnemyOgre : public EnemyBase
{
	// アニメーション
	enum class ANIM_TYPE
	{
		IDLE,			// 待機
		WALK,			// 歩く
		RUN,			// 走る
		FAST_RUN,		// 走り始め
		PREV_ATTACK_01,	// 攻撃前
		PREV_ATTACK_02,	// 攻撃前
		ATTACK_1,		// 攻撃１
		ATTACK_2,		// 攻撃２
		ATTACK_3,		// 攻撃３
		ATTACK_4,		// 攻撃４
		DASHATTACK,		// ダッシュ攻撃
		AVOID,			// 回避
		DISCOVERY,		// 追跡対象の発見
		LOSS,			// 追跡対象の喪失
		DAMAGE_SMALL,	// 少量のダメージ
		DIE				// 死亡
		//MAX,
	};

public:
	EnemyOgre(void);
	virtual ~EnemyOgre(void) override;

	void Release(void)override;

	// 攻撃判定は、手に持った斧のコライダを使う
	const ColliderBase* GetAttackCollider(void) const override;

	// 攻撃のダメージ量は、斧の値（weapon_param.csv の AXE）を使う
	int GetAttackDamage(void) const override;

protected:

	float walkTimer_;

	// 斧のモデルハンドル
	int axeHandle_;

	// 斧を右手(RIGHT_HAND)に装備する（装着フレームなどは weapon_param.csv の AXE 行）
	void EquipAxe(void);

	void InitLoad(void) override;
	void InitTransform(void) override;
	void InitCollider(void) override;
	void InitAnimation(void) override;
	void InitPost(void) override;

	// 各ステートの更新処理
	void UpdateIdle(void) override;
	void UpdateDiscovery(void) override;
	void UpdateDamage(void) override;
	void UpdateWalk(void) override;
	void UpdateRun(void) override;
	void UpdateAttack(void) override;
	void UpdatePrevAttack(void) override;
	void UpdateDie(void) override;
	void UpdateEnd(void) override;

	// ステート遷移関数
	void ChangeState(STATE newState) override;

	bool AttackCircle(const VECTOR& playerPos)const;
};

