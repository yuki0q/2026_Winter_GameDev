#pragma once
#include <memory>
#include <vector>
#include <DxLib.h>
#include "SlashProjectile.h"

class EnemyBase;

// 飛び道具の管理（生成・更新・描画・敵との当たり判定）
class ProjectileManager
{
public:

	ProjectileManager(void);
	~ProjectileManager(void);

	void Init(void);
	void Update(void);
	void Draw(void);
	void Release(void);

	// 斬撃を発射（パラメータは player_slash.csv の値）
	void SpawnSlash(const VECTOR& pos, const VECTOR& dir);

	// 飛び道具と敵の当たり判定（当たった敵にダメージを与える。同じ飛び道具が同じ敵に当たるのは1回だけ）
	void CollisionToEnemies(const std::vector<std::shared_ptr<EnemyBase>>& enemies);

private:

	std::vector<std::shared_ptr<SlashProjectile>> projectiles_;

	// 当たり判定の表示（1キーで切り替え）
	bool isDebug_;

	void UpdateDebug(void);

};
