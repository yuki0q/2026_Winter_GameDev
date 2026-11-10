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
        std::shared_ptr<Player> player, const VECTOR& spawnPos, bool isActive);

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

    // 出現条件を満たして、ゲーム開始後に出現した敵に対して呼ばれる処理
    // （プレイヤー側の衝突対象への登録など、GameScene 側でしか行えない設定に使う）
    using AppearCallback = std::function<void(const std::shared_ptr<EnemyBase>&)>;
    void SetOnEnemyAppear(AppearCallback callback) { onEnemyAppear_ = std::move(callback); }

    // 出現条件待ちの敵がまだ残っているか
    bool HasPendingSpawn(void) const { return !pendingSpawns_.empty(); }

private:

    std::vector<std::shared_ptr<EnemyBase>> enemies_;

    // 敵が倒された時の通知先
    DeadCallback onEnemyDead_;

    // 条件付きで出現した敵の通知先
    AppearCallback onEnemyAppear_;

    // 出現条件待ちの敵（waitType の敵がすべて倒されたら、type の敵が pos に出現する）
    struct PendingSpawn
    {
        std::shared_ptr<EnemyBase> enemy; // 生成して待機させておく敵
        EnemyBase::ENEMY_TYPE waitType;   // この種類の敵が全滅するのを待つ
    };
    std::vector<PendingSpawn> pendingSpawns_;

    // 条件付きで出現させる時に使うプレイヤー
    std::shared_ptr<Player> player_;

    // AddHitCollider で登録された衝突対象（後から出現する敵にも同じ設定をするため保持する）
    std::vector<const ColliderBase*> hitColliders_;

    // 出現条件の判定と、条件を満たした敵の出現
    void UpdatePendingSpawns(void);

    // 指定した種類の敵が、生存も出現待ちもしていないか
    bool IsTypeAllDead(EnemyBase::ENEMY_TYPE type) const;
};
