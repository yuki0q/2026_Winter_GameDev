#include "EnemyManager.h"
#include "EnemyAlter.h"
#include "EnemyOgre.h" 

EnemyManager::EnemyManager(void)
{
}

EnemyManager::~EnemyManager(void)
{
}

void EnemyManager::Init(void)
{
    Release();
}

void EnemyManager::Update(void)
{
    // 各敵の更新処理
    for (auto& enemy : enemies_)
    {
        if (enemy)
        {
            enemy->Update();
        }
    }

    // 死亡した敵を自動的にリストから除外・解放（std::remove_if）
    enemies_.erase(
        std::remove_if(enemies_.begin(), enemies_.end(),
            [this](const std::shared_ptr<EnemyBase>& enemy) {
                if (enemy && enemy->IsDead())
                {
                    // 解放する前に、倒された敵の種類と位置を通知する（ドロップ処理など）
                    if (onEnemyDead_)
                    {
                        onEnemyDead_(enemy->GetEnemyType(), enemy->GetTransform().pos);
                    }

                    enemy->Release();
                    return true;
                }
                return false;
            }),
        enemies_.end()
    );
}

void EnemyManager::Draw(void)
{
    for (const auto& enemy : enemies_)
    {
        if (enemy && !enemy->IsDead())
        {
            enemy->Draw();
        }
    }
}

void EnemyManager::Release(void)
{
    for (auto& enemy : enemies_)
    {
        if (enemy)
        {
            enemy->Release();
        }
    }
    enemies_.clear();
}

void EnemyManager::AddHitCollider(const ColliderBase* hitCollider)
{
    // 敵にコライダーを設定
    for (auto& enemy : enemies_)
    {
        if (enemy)
        {
            enemy->AddHitCollider(hitCollider);
        }
    }
}

bool EnemyManager::IsAllEnemyDead(void) const
{
    if (enemies_.empty())
    {
        return true; // または true（ステージの仕様に合わせて調整）
    }

    for (const auto& enemy : enemies_)
    {
        if (enemy)
        {
            if (enemy && !enemy->IsDead())
            {
                // 生きている敵が1体でもいれば false
                return false;
            }
        }
    }

    // すべての敵が死亡している
    return true;
}

void EnemyManager::SpawnFromCsv(std::shared_ptr<Player> player)
{
    for (const auto& spawn : EnemyDataTable::GetSpawns())
    {
        EnemyBase::ENEMY_TYPE type;
        if (!EnemyBase::NameToType(spawn.type, type))
        {
            // 未知の敵種別はスキップ
            continue;
        }
        SpawnEnemy(type, player, spawn.pos);
    }
}

std::shared_ptr<EnemyBase> EnemyManager::SpawnEnemy(EnemyBase::ENEMY_TYPE type, 
    std::shared_ptr<Player> player, const VECTOR& spawnPos)
{
    std::shared_ptr<EnemyBase> newEnemy = nullptr;

    switch (type)
    {
    case EnemyBase::ENEMY_TYPE::ALTER:
    {
        auto alter = std::make_shared<EnemyAlter>();
        alter->SetPlayer(player);
        newEnemy = alter;
        break;
    }
    case EnemyBase::ENEMY_TYPE::OGRE:
    {
        auto ogre = std::make_shared<EnemyOgre>();
        ogre->SetPlayer(player);
        newEnemy = ogre;
        break;
    }
    default:
        break;
    }

    if (newEnemy)
    {
        newEnemy->SetPosition(spawnPos);
        newEnemy->Init(); // リソース・コライダ等の初期化実行

        enemies_.push_back(newEnemy);
    }

    return newEnemy;
}