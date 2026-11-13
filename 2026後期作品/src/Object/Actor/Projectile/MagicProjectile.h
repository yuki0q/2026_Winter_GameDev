#pragma once
#include <DxLib.h>
#include <memory>
#include "../../Common/Transform.h"
#include "../../Collider/ColliderCapsule.h"
#include "../Charactor/Enemy/EnemyDataTable.h"

// 敵が撃つ魔法（プレイヤーに向かってまっすぐ飛んでいく光球）
// 上下方向にも飛ぶ（狙った高さへ向かう）。プレイヤーに当たると消える
class MagicProjectile
{
public:

	MagicProjectile(const VECTOR& pos, const VECTOR& dir, int damage, const EnemyMagicParam& param);
	~MagicProjectile(void);

	// コライダが Transform のポインタを持つため、コピー・移動はできない
	MagicProjectile(const MagicProjectile&) = delete;
	MagicProjectile& operator=(const MagicProjectile&) = delete;

	void Update(void);
	void Draw(void);
	void DrawDebug(void);

	// 飛距離に達した、またはプレイヤーに当たって消えたか
	bool IsDead(void) const { return isDead_; }

	// 消す（プレイヤーに当たった時など）
	void Kill(void) { isDead_ = true; }

	int GetDamage(void) const { return damage_; }
	const VECTOR& GetPos(void) const { return transform_.pos; }
	const ColliderCapsule* GetCapsule(void) const { return collider_.get(); }

private:

	// 終盤、飛距離の何割から薄くして消えるか
	static constexpr float FADE_START_RATE = 0.85f;

	// 光球の見た目：外側の光と、中心の核の大きさ（当たり判定の半径に対する比率）
	static constexpr float GLOW_RATE = 1.7f;
	static constexpr float CORE_RATE = 0.7f;

	// 脈打つ速さと大きさ
	static constexpr float PULSE_SPEED = 18.0f;
	static constexpr float PULSE_SIZE = 0.12f;

	EnemyMagicParam param_;
	int damage_;

	// コライダ追従用（モデルは持たない）
	Transform transform_;
	std::unique_ptr<ColliderCapsule> collider_;

	// 進行方向（正規化済み。上下方向を含む）
	VECTOR dir_;

	// ここまでに飛んだ距離と、経過時間
	float traveled_;
	float age_;
	bool isDead_;

};
