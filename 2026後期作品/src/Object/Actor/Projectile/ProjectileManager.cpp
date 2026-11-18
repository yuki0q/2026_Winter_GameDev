#include <algorithm>
#include "../../../Manager/InputManager.h"
#include "../Charactor/Enemy/EnemyBase.h"
#include "../Charactor/Player.h"
#include "../../Collider/ColliderCapsule.h"
#include "ProjectileManager.h"

ProjectileManager::ProjectileManager(void)
	:
	isDebug_(false)
{
}

ProjectileManager::~ProjectileManager(void)
{
}

void ProjectileManager::Init(void)
{
	Release();
}

void ProjectileManager::Update(void)
{

	for (auto& projectile : projectiles_)
	{
		if (projectile)
		{
			projectile->Update();
		}
	}

	// 飛距離に達して消えたものをリストから除外
	projectiles_.erase(
		std::remove_if(projectiles_.begin(), projectiles_.end(),
			[](const std::shared_ptr<SlashProjectile>& projectile) {
				return !projectile || projectile->IsDead();
			}),
		projectiles_.end()
	);

	// 敵の魔法
	for (auto& magic : magics_)
	{
		if (magic)
		{
			magic->Update();
		}
	}

	// 飛距離に達した・当たって消えた魔法をリストから除外
	magics_.erase(
		std::remove_if(magics_.begin(), magics_.end(),
			[](const std::shared_ptr<MagicProjectile>& magic) {
				return !magic || magic->IsDead();
			}),
		magics_.end()
	);

	UpdateDebug();

}

void ProjectileManager::Draw(void)
{

	for (const auto& projectile : projectiles_)
	{
		if (!projectile) continue;

		projectile->Draw();

		if (isDebug_)
		{
			projectile->DrawDebug();
		}
	}

	for (const auto& magic : magics_)
	{
		if (!magic) continue;

		magic->Draw();

		if (isDebug_)
		{
			magic->DrawDebug();
		}
	}

}

void ProjectileManager::Release(void)
{
	projectiles_.clear();
	magics_.clear();
}

void ProjectileManager::SpawnSlash(const VECTOR& pos, const VECTOR& dir)
{
	projectiles_.push_back(std::make_shared<SlashProjectile>(
		pos, dir, PlayerDataTable::GetSlashParam()));
}

void ProjectileManager::CollisionToEnemies(
	const std::vector<std::shared_ptr<EnemyBase>>& enemies)
{

	const int capsuleType = static_cast<int>(CharactorBase::COLLIDER_TYPE::CAPSULE);

	for (auto& projectile : projectiles_)
	{
		if (!projectile || projectile->IsDead()) continue;

		const ColliderCapsule* projectileCapsule = projectile->GetCapsule();
		if (!projectileCapsule) continue;

		for (const auto& enemy : enemies)
		{
			// 出現待ち（非アクティブ）の敵には当てない（見えない敵に当たって、斬撃が消費されるのを防ぐ）
			if (!enemy || enemy->IsDead() || !enemy->IsActive()) continue;

			// 死亡中の敵には当てない（死亡モーションのやり直しを防ぐ）
			EnemyBase::STATE state = enemy->GetCurrentState();
			if (state == EnemyBase::STATE::DIE || state == EnemyBase::STATE::END) continue;

			// ダメージを受けた直後(DAMAGEステート)の敵は、ダメージを受け付けないので、
			// 当たった事にせず（履歴に残さず）、次のフレーム以降に改めて判定する
			if (state == EnemyBase::STATE::DAMAGE) continue;

			// 同じ敵には1回だけ当たる
			const void* key = enemy.get();
			if (projectile->HasHit(key)) continue;

			const ColliderCapsule* enemyCapsule =
				dynamic_cast<const ColliderCapsule*>(enemy->GetOwnCollider(capsuleType));
			if (!enemyCapsule || !projectileCapsule->IsHit(enemyCapsule)) continue;

			projectile->AddHit(key);
			enemy->OnDamage(projectile->GetDamage(), projectile->GetPos());
		}
	}

}

void ProjectileManager::SpawnMagic(const VECTOR& pos, const VECTOR& dir, int damage,
	const EnemyMagicParam& param)
{
	magics_.push_back(std::make_shared<MagicProjectile>(pos, dir, damage, param));
}

std::vector<ProjectileManager::PlayerHitEvent> ProjectileManager::CollisionToPlayer(
	const std::shared_ptr<Player>& player)
{

	std::vector<PlayerHitEvent> events;

	if (!player || magics_.empty())
	{
		return events;
	}

	// 回避中・被ダメージ中などは、魔法はすり抜ける
	if (player->IsInvincible())
	{
		return events;
	}

	const ColliderCapsule* playerCapsule = dynamic_cast<const ColliderCapsule*>(
		player->GetOwnCollider(static_cast<int>(CharactorBase::COLLIDER_TYPE::CAPSULE)));
	if (!playerCapsule)
	{
		return events;
	}

	for (auto& magic : magics_)
	{
		if (!magic || magic->IsDead()) continue;

		const ColliderCapsule* magicCapsule = magic->GetCapsule();
		if (!magicCapsule || !magicCapsule->IsHit(playerCapsule)) continue;

		PlayerHitEvent hit;
		hit.pos = magic->GetPos();
		hit.damage = magic->GetDamage();

		if (player->IsParry())
		{
			// パリィ成功：魔法を消す
			player->ClearParry();
			hit.isParried = true;
		}
		else
		{
			// ダメージを与える
			player->OnDamage(hit.damage, hit.pos);
			hit.isParried = false;
		}

		magic->Kill();
		events.push_back(hit);

		// ダメージを受けた直後は無敵になるので、同じフレームの残りの魔法はすり抜けさせる
		// （パリィの場合も、1フレームで1つまで）
		break;
	}

	return events;

}

void ProjectileManager::UpdateDebug(void)
{
	auto const& ins = InputManager::GetInstance();
	if (ins.IsTrgDown(KEY_INPUT_1))
	{
		isDebug_ = !isDebug_;
	}
}
