#pragma once
#include "EnemyBase.h"

class EnemyWizard : public EnemyBase
{
	// アニメーション
	enum class ANIM_TYPE
	{
		IDLE,			// 待機
		WALK,			// 歩く（距離を取る時）
		RUN,			// 走る（近づく時）
		PREV_ATTACK,	// 魔法の準備
		ATTACK_1,		// 魔法の発射
		DISCOVERY,		// 追跡対象の発見
		DAMAGE_SMALL,	// 少量のダメージ
		DIE				// 死亡
	};

public:

	EnemyWizard(void);
	virtual ~EnemyWizard(void) override;

	// 魔法で攻撃する敵は、近接用の攻撃コライダを持たない（魔法は ProjectileManager が判定する）
	const ColliderBase* GetAttackCollider(void) const override { return nullptr; }

	// 基本ダメージは魔法のダメージ（EnemyMagic.csv）
	int GetAttackDamage(void) const override { return magicParam_.damage; }

protected:

	// モデルの向きの補正（モデルによって変わるので、実際の見え方に合わせて調整）
	static constexpr VECTOR ENEMY_DEFAULT_ROT_LOCAL = { 0.0f, 180.0f * DX_PI_F / 180.0f, 0.0f };

	// プレイヤーを狙う高さ（プレイヤーの足元から）
	static constexpr float PLAYER_AIM_HEIGHT = 90.0f;

	// 距離を取る動きを続ける最大時間（秒）。壁際で下がれない時に、いつまでも撃たなくなるのを防ぐ
	static constexpr float RETREAT_MAX_TIME = 2.0f;

	// 距離を取り終えたと見なす距離の倍率（retreatDistance に対して。行き来を繰り返さないための余裕）
	static constexpr float RETREAT_END_RATE = 1.3f;

	// 魔法のパラメータ（EnemyMagic.csv）
	EnemyMagicParam magicParam_;

	// 次に魔法を撃てるまでの残り時間（秒）
	float castTimer_;

	// 現在のステートに入ってからの経過時間（秒）
	float stateTimer_;

	// RUN 中に、その場で構えている（待機アニメを再生している）か
	bool isHolding_;

	void InitLoad(void) override;
	void InitTransform(void) override;
	void InitCollider(void) override;
	void InitAnimation(void) override;
	void InitPost(void) override;

	// 更新後に、クールタイムなどの時間を進める
	void UpdateProcessPost(void) override;

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

	// プレイヤーまでの水平距離を返し、プレイヤーへの水平方向の単位ベクトルを outDir に返す
	float GetPlayerDistXZ(VECTOR& outDir) const;

	// 構える(待機アニメ) / 動く(走りアニメ) の切り替え
	void SetHolding(bool isHolding);

	// 魔法を発射する
	void FireMagic(void);
};

