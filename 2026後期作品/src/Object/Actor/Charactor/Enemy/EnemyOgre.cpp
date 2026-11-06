#include "EnemyOgre.h"
#include "../../../../Manager/ResourceManager.h"
#include "../../../../Manager/SceneManager.h"
#include "../../../../Application.h"
#include "../../../Collider//ColliderCapsule.h"
#include "../../../Collider/ColliderLine.h"
#include "../../../../Utility/AsoUtility.h"
#include "../../../../Effect/EffekseerEffect.h"
#include "../Player.h"
#include "EnemyDataTable.h"
#include "../../Weapon/WeaponManager.h"
#include "../../Weapon/WeaponAxe.h"
#include <map>

EnemyOgre::EnemyOgre(void)
    :
    EnemyBase(ENEMY_TYPE::OGRE),
    walkTimer_(),
    axeHandle_(-1)
{
}

EnemyOgre::~EnemyOgre(void)
{
}

void EnemyOgre::Release(void)
{
    // 武器(コライダ含む)を解放してから、斧のモデルを削除
    EnemyBase::Release();

    if (axeHandle_ != -1)
    {
        MV1DeleteModel(axeHandle_);
        axeHandle_ = -1;
    }
}

const ColliderBase* EnemyOgre::GetAttackCollider(void) const
{
    if (!weaponManager_)
    {
        return nullptr;
    }

    const auto& axe = weaponManager_->GetWeapon(WEAPON_SLOT::RIGHT_HAND);
    return axe ? axe->GetCollider() : nullptr;
}

int EnemyOgre::GetAttackDamage(void) const
{
    if (!weaponManager_)
    {
        return EnemyBase::GetAttackDamage();
    }

    const auto& axe = weaponManager_->GetWeapon(WEAPON_SLOT::RIGHT_HAND);
    return axe ? axe->GetDamage() : EnemyBase::GetAttackDamage();
}

void EnemyOgre::EquipAxe(void)
{
    if (axeHandle_ == -1)
    {
        return;
    }

    auto axe = std::make_shared<WeaponAxe>();

    // 装着フレーム・スロット・位置・回転・攻撃判定は weapon_param.csv の AXE 行
    const WeaponParam wp = PlayerDataTable::GetWeaponParam(WeaponTypeToName(WEAPON_TYPE::AXE));

    weaponManager_->EquipWeapon(
        WEAPON_SLOT::RIGHT_HAND,
        axe,
        axeHandle_, wp.attachFrame,
        VGet(wp.scale, wp.scale, wp.scale),
        wp.localPos, wp.localRot
    );
}

void EnemyOgre::InitLoad(void)
{
    transform_.SetModel(resMng_.LoadModelDuplicate(ResourceManager::SRC::MDL_OGRE));

    // 武器管理と斧のモデル
    weaponManager_ = std::make_shared<WeaponManager>();
    axeHandle_ = resMng_.LoadModelDuplicate(ResourceManager::SRC::MDL_AXE);
}

void EnemyOgre::InitTransform(void)
{
    transform_.scl = { param_.scale, param_.scale, param_.scale };
    //transform_.pos = { 0.0f, 100.0f, 1000.0f };
    transform_.quaRotLocal = Quaternion::Mult(transform_.quaRotLocal,
        Quaternion::AngleAxis(AsoUtility::Deg2RadF(180.0f), AsoUtility::AXIS_Y));
    InitFacing();
    transform_.Update();

    // モデルが確定した後に、武器の所有者モデルを登録して斧を装備
    weaponManager_->Init(transform_.modelId);
    EquipAxe();
}

void EnemyOgre::InitCollider(void)
{
    // 線分・カプセルコライダの初期化
    ColliderLine* colLine = new ColliderLine(
        ColliderBase::TAG::ENEMY, &transform_,
        VGet(0.0f, param_.lineTopY, 0.0f), AsoUtility::VECTOR_ZERO);
    ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::LINE), colLine);

    ColliderCapsule* colCapsule = new ColliderCapsule(
        ColliderBase::TAG::ENEMY, &transform_,
        VGet(0.0f, param_.capsuleTopY, 0.0f), VGet(0.0f, param_.capsuleDownY, 0.0f),
        param_.capsuleRadius);
    ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::CAPSULE), colCapsule);

    // 攻撃判定に使うフレーム番号と半径はCSVの値
    attackStartFrameIndex_ = param_.attackStartFrame;
    attackEndFrameIndex_ = param_.attackEndFrame;
}

void EnemyOgre::InitAnimation(void)
{
    animController_ = std::make_shared<AnimationController>(transform_.modelId);

    // CSV上の名前と ANIM_TYPE の対応表（新しいアニメを増やす時はここに追記）
    static const std::map<std::string, ANIM_TYPE> NAME_TO_ANIM = {
        { "IDLE",         ANIM_TYPE::IDLE },
        { "WALK",         ANIM_TYPE::WALK },
        { "RUN",          ANIM_TYPE::RUN },
        { "PREV_ATTACK",  ANIM_TYPE::PREV_ATTACK_01 },
        { "PREV_ATTACK_02",  ANIM_TYPE::PREV_ATTACK_02 },
        { "ATTACK_1",     ANIM_TYPE::ATTACK_1 },
        { "ATTACK_2",     ANIM_TYPE::ATTACK_2 },
        { "DISCOVERY",    ANIM_TYPE::DISCOVERY },
        { "DAMAGE_SMALL", ANIM_TYPE::DAMAGE_SMALL },
        { "DIE",          ANIM_TYPE::DIE },
    };

    for (const auto& anim : EnemyDataTable::GetAnims(EnemyBase::TypeToName(enemyType_)))
    {
        auto it = NAME_TO_ANIM.find(anim.name);
        if (it == NAME_TO_ANIM.end())
        {
            // CSVに書いたが対応表に無い名前は無視
            continue;
        }

        animController_->Add(static_cast<int>(it->second), anim.speed,
            Application::PATH_MODEL + anim.path);
    }

    animController_->Play(static_cast<int>(ANIM_TYPE::IDLE), true);
}

void EnemyOgre::InitPost(void)
{
    EffekseerEffect::GetInstance()->CreateInstance();
    EffekseerEffect::GetInstance()->Init();

    hp_ = hpMax_ = param_.hp;
    ChangeState(STATE::IDLE);
}

void EnemyOgre::ChangeState(STATE state)
{
    currentState_ = state;

    switch (currentState_)
    {
    case STATE::IDLE:
        animController_->Play(static_cast<int>(ANIM_TYPE::IDLE), true);
        isInitBaseForward_ = false;
        searchAngleTimer_ = 0.0f;
        break;
    case STATE::DISCOVERY:
        animController_->Play(static_cast<int>(ANIM_TYPE::DISCOVERY), true);
        isInitBaseForward_ = false;
        searchAngleTimer_ = 0.0f;
        break;
    case STATE::DAMAGE:
        animController_->Play(static_cast<int>(ANIM_TYPE::DAMAGE_SMALL), false);
        break;
    case STATE::WALK:
        animController_->Play(static_cast<int>(ANIM_TYPE::WALK), true);
        break;
    case STATE::RUN:
        animController_->Play(static_cast<int>(ANIM_TYPE::RUN), true);
        break;
    case STATE::PREV_ATTACK:
        attackStartFrameIndex_ = param_.attackStartFrame;
        attackEndFrameIndex_ = param_.attackEndFrame;

        attackPattern_ = GetRand(1);

        // 取得した座標にエフェクトを発生させる
        EffekseerEffect::GetInstance()->AttackLightEffect(
            MV1GetFramePosition(transform_.modelId, attackEndFrameIndex_),
            { 10.0f,10.0f,10.0f }, AsoUtility::VECTOR_ZERO);
        
        if (attackPattern_ == 0)
        {
            animController_->Play(static_cast<int>(ANIM_TYPE::PREV_ATTACK_01), false);
        }
        else
        {
            animController_->Play(static_cast<int>(ANIM_TYPE::PREV_ATTACK_02), false);
        }
        break;
    case STATE::ATTACK:
        if (attackPattern_ == 0)
        {
            attackMotion_ = "ATTACK_1";
            animController_->Play(static_cast<int>(ANIM_TYPE::ATTACK_1), false);
        }
        else
        {
            attackMotion_ = "ATTACK_2";
            animController_->Play(static_cast<int>(ANIM_TYPE::ATTACK_2), false);
        }
        break;
    case STATE::DIE:
        animController_->Play(static_cast<int>(ANIM_TYPE::DIE), false);
        break;
    case STATE::END:
        break;
    }
}

void EnemyOgre::UpdateIdle(void)
{
    VECTOR playerPos = player_->GetTransform().pos;

    // 視界内にプレイヤーが入ったかチェック
    if (SearchCircle(playerPos))
    {
        // 追跡状態へ遷移
        ChangeState(STATE::DISCOVERY);
    }

    //if (!SearchCircle(playerPos))
    //{
    //    
    //    ChangeState(STATE::WALK);
    //}
}

void EnemyOgre::UpdateDiscovery(void)
{
    SerachOscillation();

    VECTOR playerPos = player_->GetTransform().pos;

    // 視界内にプレイヤーが入ったかチェック
    if (SearchPlayer(playerPos))
    {
        // 追跡状態へ遷移
        ChangeState(STATE::RUN);
    }

    if (!SearchCircle(playerPos))
    {
        // 追跡状態へ遷移
        ChangeState(STATE::IDLE);
    }
}

void EnemyOgre::UpdateDamage(void)
{
    VECTOR playerPos = player_->GetTransform().pos;

    // ダメージモーションが終了したら、状況に応じてステートを戻す
    if (animController_->IsEnd())
    {
        if (AttackCircle(playerPos))
        {
            ChangeState(STATE::PREV_ATTACK);
        }
        else if (SearchCircle(playerPos))
        {
            ChangeState(STATE::RUN);
        }
        else
        {
            AfterChase(playerPos);
            ChangeState(STATE::IDLE);
        }
    }
}

void EnemyOgre::UpdateWalk(void)
{
    VECTOR playerPos = player_->GetTransform().pos;
    /* moveDir_ = VSub(playerPos, transform_.pos);
     moveDir_.y = 0.0f;
     transform_.rot = AsoUtility::VNormalize(moveDir_);
     movePow_ = VScale(transform_.rot, 3.0f);*/

     // 視界内にプレイヤーが入ったかチェック
    if (SearchPlayer(playerPos))
    {
        // 追跡状態へ遷移
        ChangeState(STATE::RUN);
    }
    walkTimer_ -= 1.0f;
    if (walkTimer_ <= 0.0f)
    {
        // 次に方向転換するまでの時間をランダムに設定（例: 120フレーム?300フレーム）
        walkTimer_ = static_cast<float>(GetRand(180) + 120);

        // 0 ? 360度（2π）のランダムな角度を生成
        float randomRad = static_cast<float>(GetRand(1000)) / 1000.0f * DX_TWO_PI_F;

        // ランダムな方向ベクトルを算出
        moveDir_ = VGet(sinf(randomRad), 0.0f, cosf(randomRad));
    }

    // 決定した方向に向きを徐々に合わせる、または即座に向かわせる
    if (!AsoUtility::EqualsVZero(moveDir_))
    {
        transform_.rot = AsoUtility::VNormalize(moveDir_);

        // 速度をつけて移動量を計算（WALK用の遅めの速度、例: 2.0f や 3.0f）
        movePow_ = VScale(moveDir_, param_.walkSpeed);
    }
}

void EnemyOgre::UpdateRun(void)
{
    VECTOR playerPos = player_->GetTransform().pos;
    moveDir_ = VSub(playerPos, transform_.pos);
    moveDir_.y = 0.0f;
    transform_.rot = AsoUtility::VNormalize(moveDir_);
    movePow_ = VScale(transform_.rot, param_.runSpeed);

    // 視界内にプレイヤーが入ったかチェック
    if (!SearchCircle(playerPos))
    {
        AfterChase(playerPos);

        ChangeState(STATE::IDLE);
    }

    // 攻撃範囲にプレイヤーが入ったかチェック
    if (AttackCircle(playerPos))
    {
        ChangeState(STATE::PREV_ATTACK);
    }
}

void EnemyOgre::UpdatePrevAttack(void)
{
    // 攻撃範囲にプレイヤーが入ったかチェック
    if (animController_->IsEnd())
    {
        ChangeState(STATE::ATTACK);
    }
}

void EnemyOgre::UpdateAttack(void)
{
    VECTOR playerPos = player_->GetTransform().pos;
    UpdateAttackCollider();

    if (animController_->IsEnd())
    {
        SetAttackColliderEnable(false);
    }

    if (!SearchCircle(playerPos) && animController_->IsEnd())
    {
        AfterChase(playerPos);
        ChangeState(STATE::IDLE);
    }

    if (SearchCircle(playerPos) && animController_->IsEnd())
    {
        AfterChase(playerPos);
        ChangeState(STATE::RUN);
    }
}

void EnemyOgre::UpdateDie(void)
{
    if (animController_->IsEnd())
    {
        ChangeState(STATE::END);
    }
}

void EnemyOgre::UpdateEnd(void)
{
    isDead_ = true;
}

bool EnemyOgre::AttackCircle(const VECTOR& playerPos) const
{
    // 敵とプレイヤーの距離を計算
    VECTOR diff = VSub(playerPos, transform_.pos);
    float distSq = VDot(diff, diff); // 2点間の距離の二乗

    // 半径の二乗と比較して効率化
    float radiusSq = param_.attackRadius * param_.attackRadius;
    if (distSq <= radiusSq)
    {
        return true; // 一定距離内に入っている
    }

    return false;
}