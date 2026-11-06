#include "EnemyAlter.h"
#include "../../../../Manager/ResourceManager.h"
#include "../../../../Manager/SceneManager.h"
#include "../../../../Application.h"
#include "../../../Collider//ColliderCapsule.h"
#include "../../../Collider/ColliderLine.h"
#include "../../../../Utility/AsoUtility.h"
#include "../../../../Effect/EffekseerEffect.h"
#include "../Player.h"
#include "EnemyDataTable.h"
#include <map>

EnemyAlter::EnemyAlter(void)
    : 
    EnemyBase(ENEMY_TYPE::ALTER),
    walkTimer_()
{
}

EnemyAlter::~EnemyAlter(void)
{
}

void EnemyAlter::Release(void)
{
    EnemyBase::Release();
}

void EnemyAlter::InitLoad(void)
{
    transform_.SetModel(resMng_.LoadModelDuplicate(ResourceManager::SRC::MDL_ALTER));
}

void EnemyAlter::InitTransform(void)
{
    transform_.scl = { param_.scale, param_.scale, param_.scale };
    //transform_.pos = { 0.0f, 100.0f, 1000.0f };
   /* transform_.quaRotLocal = Quaternion::Mult(transform_.quaRotLocal,
        Quaternion::AngleAxis(AsoUtility::Deg2RadF(180.0f), AsoUtility::AXIS_Y));*/
    transform_.quaRotLocal = Quaternion::Identity();
    transform_.quaRotLocal = Quaternion::Euler(ENEMY_DEFAULT_ROT_LOCAL);
    InitFacing();
    transform_.Update();    
}

void EnemyAlter::InitCollider(void)
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

    attackCollider_ = std::make_unique<ColliderCapsule>(
        ColliderBase::TAG::WEAPON, &attackTransform_,
        AsoUtility::VECTOR_ZERO, AsoUtility::VECTOR_ZERO, param_.attackColRadius);
}

void EnemyAlter::InitAnimation(void)
{
    animController_ = std::make_shared<AnimationController>(transform_.modelId);

        // CSV上の名前と ANIM_TYPE の対応表（新しいアニメを増やす時はここに追記）
    static const std::map<std::string, ANIM_TYPE> NAME_TO_ANIM = {
        { "IDLE",         ANIM_TYPE::IDLE },
        { "WALK",         ANIM_TYPE::WALK },
        { "RUN",          ANIM_TYPE::RUN },
        { "PREV_ATTACK",  ANIM_TYPE::PREV_ATTACK },
        { "ATTACK_1",     ANIM_TYPE::ATTACK_1 },
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

void EnemyAlter::InitPost(void)
{
    EffekseerEffect::GetInstance()->CreateInstance();
    EffekseerEffect::GetInstance()->Init();

    hp_ = hpMax_ = param_.hp;
    ChangeState(STATE::IDLE);
}

void EnemyAlter::ChangeState(STATE state)
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
        walkTimer_ = 0.0f;
		animController_->Play(static_cast<int>(ANIM_TYPE::WALK), true);
		break;
	case STATE::RUN:
		animController_->Play(static_cast<int>(ANIM_TYPE::RUN), true);
		break;
	case STATE::PREV_ATTACK:
        attackStartFrameIndex_ = param_.attackStartFrame;
        attackEndFrameIndex_ = param_.attackEndFrame;
        // 取得した座標にエフェクトを発生させる
        EffekseerEffect::GetInstance()->AttackLightEffect(
            MV1GetFramePosition(transform_.modelId, attackEndFrameIndex_),
            { 10.0f,10.0f,10.0f }, AsoUtility::VECTOR_ZERO);
		animController_->Play(static_cast<int>(ANIM_TYPE::PREV_ATTACK), false);
		break;
	case STATE::ATTACK:
		animController_->Play(static_cast<int>(ANIM_TYPE::ATTACK_1), false);
		break;
	case STATE::DIE:
		animController_->Play(static_cast<int>(ANIM_TYPE::DIE), false);
		break;
	case STATE::END:
		break;
	}
}

void EnemyAlter::UpdateIdle(void)
{
    VECTOR playerPos = player_->GetTransform().pos;

    // 視界内にプレイヤーが入ったかチェック
    if (SearchCircle(playerPos))
    {
        // 追跡状態へ遷移
        ChangeState(STATE::DISCOVERY);
    }

    if (!SearchCircle(playerPos))
    {
        
        ChangeState(STATE::WALK);
    }
}

void EnemyAlter::UpdateDiscovery(void)
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

void EnemyAlter::UpdateDamage(void)
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

void EnemyAlter::UpdateWalk(void)
{
    VECTOR playerPos = player_->GetTransform().pos;
   /* moveDir_ = VSub(playerPos, transform_.pos);
    moveDir_.y = 0.0f;
    transform_.rot = AsoUtility::VNormalize(moveDir_);
    movePow_ = VScale(transform_.rot, 3.0f);*/
   
    // 視界内にプレイヤーが入ったかチェック
    if (SearchCircle(playerPos))
    {
        // 追跡状態へ遷移
        ChangeState(STATE::DISCOVERY);
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

void EnemyAlter::UpdateRun(void)
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

void EnemyAlter::UpdatePrevAttack(void)
{
    // 攻撃範囲にプレイヤーが入ったかチェック
    if (animController_->IsEnd())
    {
        ChangeState(STATE::ATTACK);
    }
}

void EnemyAlter::UpdateAttack(void)
{
    VECTOR playerPos = player_->GetTransform().pos;
    UpdateAttackCollider();

    if (animController_->IsEnd()) 
    { SetAttackColliderEnable(false); }

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

void EnemyAlter::UpdateDie(void)
{
    if (animController_->IsEnd())
    {
        ChangeState(STATE::END);
    }
}

void EnemyAlter::UpdateEnd(void)
{
    isDead_ = true;
}

bool EnemyAlter::AttackCircle(const VECTOR& playerPos) const
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