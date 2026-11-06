#pragma once
#include <DxLib.h>
#include <memory>
#include <set>
#include "../../Common/Transform.h"
#include "../../Collider/ColliderCapsule.h"
#include "../Charactor/PlayerDataTable.h"

// 斬撃（まっすぐ飛んでいく三日月型の飛び道具）
// 当たり判定は、進行方向と直角（横向き）のカプセル
class SlashProjectile
{
public:

	SlashProjectile(const VECTOR& pos, const VECTOR& dir, const PlayerSlashParam& param);
	~SlashProjectile(void);

	// コライダが Transform のポインタを持つため、コピー・移動はできない
	SlashProjectile(const SlashProjectile&) = delete;
	SlashProjectile& operator=(const SlashProjectile&) = delete;

	void Update(void);
	void Draw(void);
	void DrawDebug(void);

	// 飛距離に達して消えたか
	bool IsDead(void) const { return isDead_; }

	int GetDamage(void) const { return param_.damage; }
	const VECTOR& GetPos(void) const { return transform_.pos; }
	const ColliderCapsule* GetCapsule(void) const { return collider_.get(); }

	// 同じ相手に何度も当たらないようにするための履歴（貫通する飛び道具用）
	bool HasHit(const void* target) const { return hitTargets_.count(target) > 0; }
	void AddHit(const void* target) { hitTargets_.insert(target); }

private:

	// 終盤、飛距離の何割から薄くして消えるか
	static constexpr float FADE_START_RATE = 0.8f;

	// 三日月の奥行き（横幅に対する比率）と、内側の曲線の比率
	static constexpr float CRESCENT_DEPTH_RATE = 0.7f;
	static constexpr float CRESCENT_INNER_RATE = 0.35f;

	// 三日月を分割する数
	static constexpr int CRESCENT_SEGMENTS = 12;

	// 厚みを出すために重ねる面の上下の間隔
	static constexpr float LAYER_GAP = 8.0f;

	PlayerSlashParam param_;

	// コライダ追従用（モデルは持たない）
	Transform transform_;
	std::unique_ptr<ColliderCapsule> collider_;

	// 進行方向（水平・正規化済み）と、それと直角な横方向
	VECTOR dir_;
	VECTOR lateral_;

	// ここまでに飛んだ距離
	float traveled_;
	bool isDead_;

	std::set<const void*> hitTargets_;

	// 三日月型の面を1枚描画（高さのずらし量を指定）
	void DrawCrescent(float offsetY, int alpha) const;

};
