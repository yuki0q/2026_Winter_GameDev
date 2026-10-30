#pragma once
#include "../CharactorBase.h"
#include "../../../Common/Transform.h"
#include "EnemyDataTable.h"
#include <memory>

class Player;
class ColliderCapsule;

class EnemyBase : public CharactorBase
{
public:

    enum class ENEMY_TYPE
    {
        ALTER,
        WIZARD,
        OGRE,
        MAX
    };

    // 状態
    enum class STATE
    {
        IDLE,
        WALK,
        RUN,
        PREV_ATTACK,
        ATTACK,
        JUMP,
        DISCOVERY,
        DAMAGE,
        DIE,
        END,
    };

    EnemyBase(ENEMY_TYPE type);
    virtual ~EnemyBase(void) override;

    virtual void Draw(void) override;
    virtual void Release(void) override;

    // 生存判定
    bool IsDead(void) const { return isDead_; }

    // ダメージ処理（派生クラスでオーバーライド可能）
    virtual void OnDamage(int damage, VECTOR attackedPos);

    // プレイヤーの参照をセットする関数
    void SetPlayer(std::shared_ptr<Player> player) { player_ = player; }

    const ENEMY_TYPE& GetEnemyType(void) const { return enemyType_; }

    // 敵種別 <-> CSV上の名前
    static std::string TypeToName(ENEMY_TYPE type);
    static bool NameToType(const std::string& name, ENEMY_TYPE& outType);

    void DrawDebug(void);

    // 攻撃判定の有効・無効を設定・取得
    void SetAttackColliderEnable(bool isEnable) { isAttackColliderEnable_ = isEnable; }
    bool IsAttackColliderEnable(void) const { return isAttackColliderEnable_; }

    // 攻撃用コライダの取得
    // 武器で攻撃する敵は、武器のコライダを返すように override する
    virtual const ColliderBase* GetAttackCollider(void) const { return attackCollider_.get(); }

    // 攻撃のダメージ量（武器で攻撃する敵は、武器のダメージを返すように override する）
    virtual int GetAttackDamage(void) const { return 10; }

    void SetPosition(VECTOR spawnPos);

    STATE GetCurrentState() const { return currentState_; }

protected:

    // CSV(enemy_param.csv)から読み込んだパラメータ
    // 索敵半径・視野角・HP・移動速度などはここから参照する
    EnemyParam param_;

    // 索敵関連の定数やパラメータ
    // 一定距離（SearchCircleの半径）
    static constexpr float SEARCH_RADIUS = 1000.0f;
    // 視界の視野角（度）
    static constexpr float SEARCH_VIEW_ANGLE = 45.0f;

    // 一定距離内に入ったか判定する（SearchCircle）
    bool SearchCircle(const VECTOR& playerPos) const;

    // 視界にプレイヤーが入ったか判別する（SearchPlayer）
    bool SearchPlayer(const VECTOR& playerPos) const;

    // 首振り
    void SerachOscillation(void);

    // 追跡後の処理
    void AfterChase(VECTOR pos);

    // 初期の向きを、スポーン位置からプレイヤーへの方向（水平）に合わせる
   // 各敵の InitTransform の中で、transform_.Update() の前に呼ぶ
    void InitFacing(void);

    ENEMY_TYPE enemyType_;

    // 更新系
    void UpdateProcess(void) override;
    virtual void UpdateProcessPost(void) override;

    virtual void UpdateIdle(void) = 0;
    virtual void UpdateDiscovery(void) = 0;
    virtual void UpdateDamage(void) = 0;
    virtual void UpdateWalk(void) = 0;
    virtual void UpdateRun(void) = 0;
    virtual void UpdatePrevAttack(void) = 0;
    virtual void UpdateAttack(void) = 0;
    virtual void UpdateDie(void) = 0;
    virtual void UpdateEnd(void) = 0;

    // ステート遷移関数
    virtual void ChangeState(STATE newState) = 0;

    // 敵の攻撃用コライダ（例としてカプセルコライダ）
    std::unique_ptr<ColliderBase> attackCollider_;
    bool isAttackColliderEnable_ = false;

    // 追従させるボーンのフレームインデックス
    int attackStartFrameIndex_ = -1;
    int attackEndFrameIndex_ = -1;

    VECTOR attackLocalPosTop_;
    VECTOR attackLocalPosDown_;

    Transform attackTransform_;

    // 攻撃用コライダの更新関数
    void UpdateAttackCollider(void);

    // 現在のステート
    STATE currentState_;

    std::shared_ptr<Player> player_;

    // 首振り用のタイマー
    float searchAngleTimer_;
    // 基準となる正面方向
    Quaternion baseQuaRot_;
    // 首振り中の視界の回転
    Quaternion currentSearchQuaRot_;
    // 基準方向の初期化フラグ
    bool isInitBaseForward_;
};