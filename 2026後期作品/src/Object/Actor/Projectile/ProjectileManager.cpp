#include <algorithm>
#include "../../../Manager/InputManager.h"
#include "../Charactor/Enemy/EnemyBase.h"
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

}

void ProjectileManager::Release(void)
{
	projectiles_.clear();
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

void ProjectileManager::UpdateDebug(void)
{
	auto const& ins = InputManager::GetInstance();
	if (ins.IsTrgDown(KEY_INPUT_1))
	{
		isDebug_ = !isDebug_;
	}
}
