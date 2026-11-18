#pragma once
#include <memory>
#include <vector>
#include <DxLib.h>
#include "SlashProjectile.h"
#include "MagicProjectile.h"

class EnemyBase;
class Player;

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

	// 敵の魔法を発射（パラメータは EnemyMagic.csv の値。damage は攻撃モーションの倍率を含んだ値）
	void SpawnMagic(const VECTOR& pos, const VECTOR& dir, int damage, const EnemyMagicParam& param);

	// 敵の魔法がプレイヤーに当たった時の結果
	struct PlayerHitEvent
	{
		bool isParried;	// パリィされたか（false ならダメージを与えた）
		VECTOR pos;		// 当たった位置
		int damage;		// 魔法のダメージ量
	};

	// 敵の魔法とプレイヤーの当たり判定
	// ・パリィ中なら魔法を消す（プレイヤーのパリィ成功扱い）
	// ・回避中などダメージを受けない間は、魔法はすり抜ける
	// ・それ以外はプレイヤーにダメージを与えて、魔法を消す
	// 当たりごとの結果を返す（演出やヒットストップは呼び出し側で行う）
	std::vector<PlayerHitEvent> CollisionToPlayer(const std::shared_ptr<Player>& player);

private:

	std::vector<std::shared_ptr<SlashProjectile>> projectiles_;

	// 敵が撃った魔法
	std::vector<std::shared_ptr<MagicProjectile>> magics_;

	// 当たり判定の表示（1キーで切り替え）
	bool isDebug_;

	void UpdateDebug(void);

};
