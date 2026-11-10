#include <Windows.h>
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
        if (!enemy) continue;
        if (!enemy->IsActive()) continue;
        enemy->Update();
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

    // 倒された敵を除外した後に、出現条件を満たした敵を出現させる
    UpdatePendingSpawns();
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

    pendingSpawns_.clear();
    hitColliders_.clear();
    player_ = nullptr;
}

void EnemyManager::AddHitCollider(const ColliderBase* hitCollider)
{
    // 後から出現する敵にも同じ設定を行えるように、登録内容を覚えておく
    hitColliders_.push_back(hitCollider);

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
    // 出現条件待ちの敵がいる間は、全滅とは見なさない
    if (!pendingSpawns_.empty())
    {
        return false;
    }

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
    // 条件付きの出現で使うため、プレイヤーを覚えておく
    player_ = player;
    pendingSpawns_.clear();

    const std::string AFTER_PREFIX = "AFTER_";

    for (const auto& spawn : EnemyDataTable::GetSpawns())
    {
        EnemyBase::ENEMY_TYPE type;
        if (!EnemyBase::NameToType(spawn.type, type))
        {
            // 未知の敵種別はスキップ
            continue;
        }

        // 出現条件なし（または START）なら、今すぐ出現
        if (spawn.condition.empty() || spawn.condition == "START")
        {
            SpawnEnemy(type, player, spawn.pos, true);
            continue;
        }

        // 「AFTER_ALTER」のように、指定した種類の敵を全滅させた後に出現
        EnemyBase::ENEMY_TYPE waitType;
        if (spawn.condition.compare(0, AFTER_PREFIX.size(), AFTER_PREFIX) == 0 &&
            EnemyBase::NameToType(spawn.condition.substr(AFTER_PREFIX.size()), waitType))
        {
            auto enemy = SpawnEnemy(type, player, spawn.pos, false); // 最初は非アクティブ
            if (enemy)
            {
                pendingSpawns_.push_back({ enemy, waitType }); // 生成済みのインスタンスと待機条件を保持
            }
            continue;
        }

        // 解釈できない条件は、出現させずに警告だけ出す
        OutputDebugStringA(("[EnemyManager] EnemySpawn.csv の出現条件を解釈できません: "
            + spawn.condition + "\n").c_str());
    }
}

std::shared_ptr<EnemyBase> EnemyManager::SpawnEnemy(EnemyBase::ENEMY_TYPE type, 
    std::shared_ptr<Player> player, const VECTOR& spawnPos, bool isActive)
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
        // リソース・コライダ等の初期化実行
        newEnemy->Init();
        // アクティブ状態を設定
        newEnemy->SetActive(isActive); 

        // これまでに登録された衝突対象（ステージなど）を、この敵にも設定する
        for (const auto* hitCollider : hitColliders_)
        {
            newEnemy->AddHitCollider(hitCollider);
        }

        enemies_.push_back(newEnemy);
    }

    return newEnemy;
}

void EnemyManager::UpdatePendingSpawns(void)
{
    if (pendingSpawns_.empty() || !player_) return;

    for (auto it = pendingSpawns_.begin(); it != pendingSpawns_.end();)
    {
        // 待っている種類の敵が、まだ残っていれば出現しない
        if (!IsTypeAllDead(it->waitType))
        {
            ++it;
            continue;
        }

        // 条件を満たしたので、あらかじめロード・生成しておいたボスを有効化（アクティブに）する！
        if (it->enemy)
        {
            it->enemy->SetActive(true);

            // 非アクティブの間は更新していないので、出現した時点で一度更新しておく
            // （武器の位置などが反映される前の姿が、1フレーム映らないようにする）
            it->enemy->Update();

            // シーン側（GameScene）などに「出現したこと」を通知するコールバックがあれば呼ぶ
            if (onEnemyAppear_)
            {
                onEnemyAppear_(it->enemy);
            }
        }

        it = pendingSpawns_.erase(it);
    }
}

bool EnemyManager::IsTypeAllDead(EnemyBase::ENEMY_TYPE type) const
{
    // まだ生きている敵がいる
    for (const auto& enemy : enemies_)
    {
        if (enemy && !enemy->IsDead() && enemy->GetEnemyType() == type)
        {
            return false;
        }
    }

    // その種類自身が出現待ちの場合も、全滅とは見なさない
    for (const auto& pending : pendingSpawns_)
    {
        // pending.enemy が存在し、その敵のタイプが一致するかチェック
        if (pending.enemy && pending.enemy->GetEnemyType() == type)
        {
            return false;
        }
    }

    return true;
}
