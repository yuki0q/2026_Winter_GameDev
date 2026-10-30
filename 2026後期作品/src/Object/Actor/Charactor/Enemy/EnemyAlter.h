#pragma once
#include "EnemyBase.h"

class EnemyAlter : public EnemyBase
{
	// アニメーション
	enum class ANIM_TYPE
	{
		IDLE,
		WALK,
		RUN,
		FAST_RUN,
		JUMP,
		PREV_ATTACK,// 攻撃前
		ATTACK_1,	// 攻撃１
		ATTACK_2,	// 攻撃２
		ATTACK_3,	// 攻撃３
		ATTACK_4,	// 攻撃４
		DASHATTACK,	// ダッシュ攻撃
		AVOID,
		DISCOVERY,	// 追跡対象の発見
		LOSS,		// 追跡対象の喪失
		DAMAGE_SMALL,
		DIE
		//MAX,
	};

public:
    EnemyAlter(void);
    virtual ~EnemyAlter(void) override;

	void Release(void)override;

protected:

	static constexpr VECTOR ENEMY_DEFAULT_ROT_LOCAL = { 0.0f,180.0f * DX_PI_F / 180.0f,0.0f };

	float walkTimer_;

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