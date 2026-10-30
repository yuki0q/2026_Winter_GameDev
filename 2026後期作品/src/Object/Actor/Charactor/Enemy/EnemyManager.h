#pragma once
#include <vector>
#include <memory>
#include <functional>
#include <utility>
#include "EnemyBase.h"

class EnemyManager
{
public:

    EnemyManager(void);
    ~EnemyManager(void);

    void Init(void);
    void Update(void);
    void Draw(void);
    void Release(void);

    // 敵の生成（スポーン）
    std::shared_ptr<EnemyBase> SpawnEnemy(EnemyBase::ENEMY_TYPE type,
        std::shared_ptr<Player> player, const VECTOR& spawnPos);

    // enemy_spawn.csv の内容に従って敵をまとめて生成
    void SpawnFromCsv(std::shared_ptr<Player> player);

    // 管理中のすべての敵リストを取得（当たり判定などで使用）
    const std::vector<std::shared_ptr<EnemyBase>>& GetEnemies(void) const { return enemies_; }

    // 衝突対象となるコライダを登録
    void AddHitCollider(const ColliderBase* hitCollider);

    bool IsAllEnemyDead(void) const;

    // 敵が倒されてリストから外れる時に呼ばれる処理（敵の種類と、倒された位置が渡される）
    // アイテムのドロップなど、敵管理の外側の処理をつなぐために使う
    using DeadCallback = std::function<void(EnemyBase::ENEMY_TYPE, const VECTOR&)>;
    void SetOnEnemyDead(DeadCallback callback) { onEnemyDead_ = std::move(callback); }

private:

    std::vector<std::shared_ptr<EnemyBase>> enemies_;

    // 敵が倒された時の通知先
    DeadCallback onEnemyDead_;
};
