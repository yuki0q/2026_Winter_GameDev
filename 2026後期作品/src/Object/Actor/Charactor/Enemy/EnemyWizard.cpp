#include "EnemyWizard.h"
#include "../../../../Manager/ResourceManager.h"
#include "../../../../Manager/SceneManager.h"
#include "../../../../Application.h"
#include "../../../Collider/ColliderCapsule.h"
#include "../../../Collider/ColliderLine.h"
#include "../../../../Utility/AsoUtility.h"
#include "../Player.h"
#include "EnemyDataTable.h"
#include <map>

EnemyWizard::EnemyWizard(void)
	:
	EnemyBase(ENEMY_TYPE::WIZARD),
	castTimer_(0.0f),
	stateTimer_(0.0f),
	isHolding_(false)
{
	// 魔法のパラメータをCSVから取得（Init()より前に確定させる）
	magicParam_ = EnemyDataTable::GetMagicParam(TypeToName(ENEMY_TYPE::WIZARD));
}

EnemyWizard::~EnemyWizard(void)
{
}

void EnemyWizard::InitLoad(void)
{
	transform_.SetModel(resMng_.LoadModelDuplicate(ResourceManager::SRC::MDL_WIZARD));
}

void EnemyWizard::InitTransform(void)
{
	transform_.scl = { param_.scale, param_.scale, param_.scale };

	transform_.quaRotLocal = Quaternion::Identity();
	transform_.quaRotLocal = Quaternion::Euler(ENEMY_DEFAULT_ROT_LOCAL);

	// 向きはプレイヤーの方向に初期化する（quaRotLocal はモデル自体の向きの補正）
	InitFacing();
	transform_.Update();
}

void EnemyWizard::InitCollider(void)
{
	// 線分・カプセルコライダの初期化（近接攻撃用のコライダは持たない）
	ColliderLine* colLine = new ColliderLine(
		ColliderBase::TAG::ENEMY, &transform_,
		VGet(0.0f, param_.lineTopY, 0.0f), AsoUtility::VECTOR_ZERO);
	ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::LINE), colLine);

	ColliderCapsule* colCapsule = new ColliderCapsule(
		ColliderBase::TAG::ENEMY, &transform_,
		VGet(0.0f, param_.capsuleTopY, 0.0f), VGet(0.0f, param_.capsuleDownY, 0.0f),
		param_.capsuleRadius);
	ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::CAPSULE), colCapsule);
}

void EnemyWizard::InitAnimation(void)
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

void EnemyWizard::InitPost(void)
{
	hp_ = hpMax_ = param_.hp;
	ChangeState(STATE::IDLE);
}

void EnemyWizard::UpdateProcessPost(void)
{
	EnemyBase::UpdateProcessPost();

	if (isDead_ || !isActive_) return;

	// クールタイムと、ステートの経過時間を進める
	const float dt = scnMng_.GetDeltaTime();
	stateTimer_ += dt;

	if (castTimer_ > 0.0f)
	{
		castTimer_ -= dt;
		if (castTimer_ < 0.0f)
		{
			castTimer_ = 0.0f;
		}
	}
}

void EnemyWizard::ChangeState(STATE state)
{
	currentState_ = state;
	stateTimer_ = 0.0f;

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
		isHolding_ = false;
		animController_->Play(static_cast<int>(ANIM_TYPE::RUN), true);
		break;
	case STATE::PREV_ATTACK:
		animController_->Play(static_cast<int>(ANIM_TYPE::PREV_ATTACK), false);
		break;
	case STATE::ATTACK:
		attackMotion_ = "ATTACK_1";
		animController_->Play(static_cast<int>(ANIM_TYPE::ATTACK_1), false);

		// 発射と同時に、次のクールタイムを開始する
		FireMagic();
		castTimer_ = magicParam_.castInterval;
		break;
	case STATE::DIE:
		animController_->Play(static_cast<int>(ANIM_TYPE::DIE), false);
		break;
	case STATE::END:
		break;
	}
}

void EnemyWizard::UpdateIdle(void)
{
	VECTOR playerPos = player_->GetTransform().pos;

	// 視界内にプレイヤーが入ったかチェック
	if (SearchCircle(playerPos))
	{
		// 追跡状態へ遷移
		ChangeState(STATE::DISCOVERY);
	}
}

void EnemyWizard::UpdateDiscovery(void)
{
	SerachOscillation();

	VECTOR playerPos = player_->GetTransform().pos;

	// 視界内にプレイヤーが入ったかチェック
	if (SearchPlayer(playerPos))
	{
		ChangeState(STATE::RUN);
	}

	if (!SearchCircle(playerPos))
	{
		ChangeState(STATE::IDLE);
	}
}

void EnemyWizard::UpdateDamage(void)
{
	VECTOR playerPos = player_->GetTransform().pos;

	// ダメージモーションが終了したら、状況に応じてステートを戻す
	if (animController_->IsEnd())
	{
		if (SearchCircle(playerPos))
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

void EnemyWizard::UpdateWalk(void)
{
	VECTOR playerPos = player_->GetTransform().pos;

	VECTOR dir;
	float dist = GetPlayerDistXZ(dir);

	// プレイヤーのほうを向いたまま、後ろへ下がる
	// （向きは moveDir_ に従って回転するので、プレイヤー方向を入れておく）
	moveDir_ = dir;
	movePow_ = VScale(dir, -param_.walkSpeed);

	if (!SearchCircle(playerPos))
	{
		AfterChase(playerPos);
		ChangeState(STATE::IDLE);
		return;
	}

	// 十分に距離を取れたら、構え直す
	if (dist >= magicParam_.retreatDistance * RETREAT_END_RATE)
	{
		ChangeState(STATE::RUN);
		return;
	}

	// 壁際などで下がりきれない時は、一定時間でその場から撃つ
	if (stateTimer_ >= RETREAT_MAX_TIME && castTimer_ <= 0.0f)
	{
		ChangeState(STATE::PREV_ATTACK);
	}
}

void EnemyWizard::UpdateRun(void)
{
	VECTOR playerPos = player_->GetTransform().pos;

	VECTOR dir;
	float dist = GetPlayerDistXZ(dir);

	// プレイヤーのほうを向く
	moveDir_ = dir;

	// 見失ったら待機へ
	if (!SearchCircle(playerPos))
	{
		AfterChase(playerPos);
		ChangeState(STATE::IDLE);
		return;
	}

	// 近づかれすぎたら、距離を取る
	if (dist < magicParam_.retreatDistance)
	{
		ChangeState(STATE::WALK);
		return;
	}

	if (dist <= param_.attackRadius)
	{
		// 射程内：その場で構えて、クールタイムが明けたら魔法の準備へ
		SetHolding(true);

		if (castTimer_ <= 0.0f)
		{
			ChangeState(STATE::PREV_ATTACK);
		}
	}
	else
	{
		// 射程外：射程に入るまで近づく
		SetHolding(false);
		movePow_ = VScale(dir, param_.runSpeed);
	}
}

void EnemyWizard::UpdatePrevAttack(void)
{
	// 準備の間も、プレイヤーのほうを向き続ける
	VECTOR dir;
	GetPlayerDistXZ(dir);
	moveDir_ = dir;

	// 準備モーションが終わったら発射
	if (animController_->IsEnd())
	{
		ChangeState(STATE::ATTACK);
	}
}

void EnemyWizard::UpdateAttack(void)
{
	VECTOR playerPos = player_->GetTransform().pos;

	VECTOR dir;
	GetPlayerDistXZ(dir);
	moveDir_ = dir;

	// 発射モーションが終わったら、状況に応じてステートを戻す
	if (animController_->IsEnd())
	{
		if (SearchCircle(playerPos))
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

void EnemyWizard::UpdateDie(void)
{
	if (animController_->IsEnd())
	{
		ChangeState(STATE::END);
	}
}

void EnemyWizard::UpdateEnd(void)
{
	isDead_ = true;
}

float EnemyWizard::GetPlayerDistXZ(VECTOR& outDir) const
{
	VECTOR diff = VSub(player_->GetTransform().pos, transform_.pos);
	diff.y = 0.0f;

	float dist = VSize(diff);
	if (dist > 0.001f)
	{
		outDir = VScale(diff, 1.0f / dist);
	}
	else
	{
		// 真上・真下などで方向が決まらない場合は、今向いている方向
		outDir = AsoUtility::VNormalize(transform_.quaRot.PosAxis(AsoUtility::DIR_F));
	}

	return dist;
}

void EnemyWizard::SetHolding(bool isHolding)
{
	// 切り替わった時だけ、アニメーションを変える（毎フレーム Play し直さない）
	if (isHolding == isHolding_)
	{
		return;
	}

	isHolding_ = isHolding;
	animController_->Play(static_cast<int>(isHolding ? ANIM_TYPE::IDLE : ANIM_TYPE::RUN), true);
}

void EnemyWizard::FireMagic(void)
{
	if (!onFireMagic_ || !player_)
	{
		return;
	}

	// 発射位置：向いている方向へ少し前、足元から castHeight の高さ
	VECTOR forward = transform_.quaRot.PosAxis(AsoUtility::DIR_F);
	forward.y = 0.0f;
	forward = AsoUtility::VNormalize(forward);

	VECTOR castPos = VAdd(transform_.pos, VScale(forward, magicParam_.castOffset));
	castPos.y += magicParam_.castHeight;

	// プレイヤーの胸のあたりを狙う（上下方向も含めて向ける）
	VECTOR target = player_->GetTransform().pos;
	target.y += PLAYER_AIM_HEIGHT;

	VECTOR dir = VSub(target, castPos);
	if (VSize(dir) < 0.001f)
	{
		dir = forward;
	}
	dir = AsoUtility::VNormalize(dir);

	// 現在のモーション(ATTACK_1)の倍率を含めたダメージで発射
	onFireMagic_(castPos, dir, CalcAttackDamage(), magicParam_);
}
