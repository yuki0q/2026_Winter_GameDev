#include "Player.h"
#include "../../../Manager/ResourceManager.h"
#include "../../../Manager/Resource.h"
#include "../../../Utility/AsoUtility.h"
#include "../../../Manager/InputManager.h"
#include "../../../Manager/SceneManager.h"
#include "../../../Manager/Camera.h"
#include "../../../Application.h"
#include "../../Collider/ColliderLine.h"
#include "../../Collider/ColliderCapsule.h"
#include "../Weapon/WeaponManager.h"
#include "../Weapon/WeaponSword.h"
#include "../Weapon/WeaponShield.h"

namespace
{
	const float WALK_SPEED = 5.0f;
	const float DASH_SPEED = 15.0f;
	const VECTOR SWORD_SCL = { 0.008f,0.008f,0.008f };
	const VECTOR SHIELD_SCL = { 0.02f ,0.02f ,0.02f };
	const VECTOR SWORD_LOCALPOS = { -0.1f ,-0.005f ,-0.08f };
	const VECTOR SWORD_LOCALROT = { DX_PI_F / 4.0f ,DX_PI_F / 12.0f ,-DX_PI_F / 3.0f };
	const VECTOR SHIELD_LOCALPOS = { 0.055f ,-0.05f ,0.05f };
	const VECTOR SHIELD_LOCALROT = { DX_PI_F / 2.0f ,DX_PI_F / 4.0f,0.0f };
}

Player::Player(void)
	:
	swordHandle_(),
	shieldHandle_(),
	isParry_(false),
	isAttack1_(false),
	currentState_(),
	isDashEnable_(true),
	attackStep_(),
	hasComboInput_(false)
{
}

Player::~Player(void)
{
}

void Player::Update(void)
{
	CharactorBase::Update();

	weaponManager_->Update();
}

void Player::Draw(void)
{
	CharactorBase::Draw();

	weaponManager_->Draw();

	DrawDegug();
}

void Player::Release(void)
{
	CharactorBase::Release();
	weaponManager_->Release();

	MV1DeleteModel(swordHandle_);
	MV1DeleteModel(shieldHandle_);
}

void Player::InitLoad(void)
{
	transform_.SetModel(resMng_.LoadModelDuplicate(
		ResourceManager::SRC::PLAYER));
	
	weaponManager_ = std::make_shared<WeaponManager>();

	swordHandle_ = resMng_.LoadModelDuplicate(
		ResourceManager::SRC::SWORD);

	shieldHandle_ = resMng_.LoadModelDuplicate(
		ResourceManager::SRC::SHIELD);
}

void Player::UpdateProcess(void)
{
	movePow_ = AsoUtility::VECTOR_ZERO;
	moveSpeed_ = 0.0f;

	// 現在のステートに応じた更新処理を実行
	switch (currentState_)
	{
	case STATE::IDLE:   
		UpdateIdle();   
		break;
	case STATE::WALK:   
		UpdateWalk();   
		break;
	case STATE::RUN:    
		UpdateRun();    
		break;
	case STATE::ATTACK: 
		UpdateAttack();
		break;
	case STATE::PARRY: 
		UpdateParry();  
		break;
	}

		/*ProcessMove();
		ProcessAttack();
		ProcessParry();*/
}

void Player::ChangeState(STATE state)
{
	currentState_ = state;

	switch (currentState_)
	{
	case STATE::IDLE:
		animController_->Play(static_cast<int>(ANIM_TYPE::IDLE), true);
		break;
	case STATE::WALK:
		animController_->Play(static_cast<int>(ANIM_TYPE::WALK), true);
		break;
	case STATE::RUN:
		animController_->Play(static_cast<int>(ANIM_TYPE::SS_RUN), true);
		break;
	case STATE::ATTACK:
		// 攻撃開始時にダッシュ入力をリセット
		isDashEnable_ = false; 
		// 先行入力フラグをクリア
		hasComboInput_ = false; 

		// 攻撃ステップに応じたアニメーションの再生
		if (attackStep_ == 0)
		{
			animController_->Play(static_cast<int>(ANIM_TYPE::SS_DASHATTACK), false);
		}
		if (attackStep_ == 1)
		{
			animController_->Play(static_cast<int>(ANIM_TYPE::SS_ATTACK_1), false);
		}
		else if (attackStep_ == 2)
		{
			animController_->Play(static_cast<int>(ANIM_TYPE::SS_ATTACK_2), false);
		}
		else if (attackStep_ == 3)
		{
			animController_->Play(static_cast<int>(ANIM_TYPE::SS_ATTACK_3), false);
		}
		else if (attackStep_ == 4)
		{
			animController_->Play(static_cast<int>(ANIM_TYPE::SS_ATTACK_4), false);
		}

		break;
	case STATE::PARRY:
		animController_->Play(static_cast<int>(ANIM_TYPE::SS_PARRY), false);
		isDashEnable_ = false;
		break;
	}
}

VECTOR Player::GetInputDirection(bool& IsDash)
{
	auto& ins = InputManager::GetInstance();
	VECTOR dir = AsoUtility::VECTOR_ZERO;
	IsDash = false;

	if (GetJoypadNum() == 0)
	{
		if (ins.IsNew(KEY_INPUT_W)) { dir = AsoUtility::DIR_F; }
		if (ins.IsNew(KEY_INPUT_A)) { dir = AsoUtility::DIR_L; }
		if (ins.IsNew(KEY_INPUT_S)) { dir = AsoUtility::DIR_B; }
		if (ins.IsNew(KEY_INPUT_D)) { dir = AsoUtility::DIR_R; }

		if (ins.IsNew(KEY_INPUT_W) && ins.IsNew(KEY_INPUT_A)) { dir = AsoUtility::DIR_FL; }
		if (ins.IsNew(KEY_INPUT_W) && ins.IsNew(KEY_INPUT_D)) { dir = AsoUtility::DIR_FR; }
		if (ins.IsNew(KEY_INPUT_S) && ins.IsNew(KEY_INPUT_A)) { dir = AsoUtility::DIR_BL; }
		if (ins.IsNew(KEY_INPUT_S) && ins.IsNew(KEY_INPUT_D)) { dir = AsoUtility::DIR_BR; }

		// ダッシュ入力のリセット判定
		if (!ins.IsNew(KEY_INPUT_LSHIFT))
		{
			// キーが一度でも離されたらダッシュ入力を再度有効化
			isDashEnable_ = true;
		}

		// フラグが true の場合のみダッシュ判定を有効にする
		if (isDashEnable_ && ins.IsNew(KEY_INPUT_LSHIFT))
		{
			IsDash = true;
		}

		if (ins.IsNew(KEY_INPUT_RETURN)) { transform_.pos = DEFAULT_POS; }
	}
	else
	{
		InputManager::JOYPAD_IN_STATE padState = ins.GetJPadInputState(InputManager::JOYPAD_NO::PAD1);
		dir = ins.GetDirectionXZAKey(padState.AKeyLX, padState.AKeyLY);

		if (!ins.IsPadBtnNew(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::R_TRIGGER))
		{
			isDashEnable_ = true;
		}

		if (isDashEnable_ && ins.IsPadBtnNew(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::R_TRIGGER))
		{
			IsDash = true;
		}
	}

	return dir;
}

void Player::MoveDirection(const VECTOR& dir, float speed)
{
	if (AsoUtility::EqualsVZero(dir)) return;

	moveSpeed_ = speed;
	Quaternion cameraRot = scnMng_.GetCamera()->GetQuaRotY();
	moveDir_ = Quaternion::PosAxis(cameraRot, dir);
	movePow_ = VScale(moveDir_, moveSpeed_);
	//transform_.pos = VAdd(transform_.pos, movePow_);
}

void Player::UpdateIdle(void)
{
	auto& ins = InputManager::GetInstance();

	// 攻撃優先（クリック）
	if (ins.IsTrgMouseLeft())
	{
		attackStep_ = 1; // 1段目から開始
		ChangeState(STATE::ATTACK);
		return;
	}

	// パリィ優先（Spaceキー）
	if (ins.IsTrgDown(KEY_INPUT_SPACE))
	{
		ChangeState(STATE::PARRY);
		return;
	}

	// 移動
	bool isDash = false;
	VECTOR dir = GetInputDirection(isDash);
	if (!AsoUtility::EqualsVZero(dir))
	{
		ChangeState(isDash ? STATE::RUN : STATE::WALK);
		return;
	}
}

void Player::UpdateWalk(void)
{
	auto& ins = InputManager::GetInstance();

	if (ins.IsTrgMouseLeft())
	{
		attackStep_ = 1; // 1段目から開始
		ChangeState(STATE::ATTACK);
		return;
	}

	if (ins.IsTrgDown(KEY_INPUT_SPACE))
	{
		ChangeState(STATE::PARRY);
		return;
	}

	bool isDash = false;
	VECTOR dir = GetInputDirection(isDash);
	if (AsoUtility::EqualsVZero(dir))
	{
		ChangeState(STATE::IDLE);
		return;
	}

	if (isDash)
	{
		ChangeState(STATE::RUN);
		return;
	}

	MoveDirection(dir, WALK_SPEED);
}

void Player::UpdateRun(void)
{
	auto& ins = InputManager::GetInstance();

	if (ins.IsTrgMouseLeft())
	{
		attackStep_ = 0; // ダッシュ攻撃から開始
		ChangeState(STATE::ATTACK);
		return;
	}

	if (ins.IsTrgDown(KEY_INPUT_SPACE))
	{
		ChangeState(STATE::PARRY);
		return;
	}

	bool isDash = false;
	VECTOR dir = GetInputDirection(isDash);
	if (AsoUtility::EqualsVZero(dir))
	{
		ChangeState(STATE::IDLE);
		return;
	}

	if (!isDash)
	{
		ChangeState(STATE::WALK);
		return;
	}

	MoveDirection(dir, DASH_SPEED);
}

void Player::UpdateAttack(void)
{	
	// 攻撃中は武器の当たり判定を有効化
	weaponManager_->SetAttackEnable(WEAPON_SLOT::RIGHT_HAND, true);

	auto& ins = InputManager::GetInstance();

	bool isDash = false;
	VECTOR dir = GetInputDirection(isDash);

	// ダッシュで攻撃をキャンセル
	if (isDash)
	{
		attackStep_ = 0;
		hasComboInput_ = false;
		ChangeState(STATE::RUN);
		return;
	}

	// 攻撃中もパリィが出せるように
	if (ins.IsTrgDown(KEY_INPUT_SPACE))
	{
		attackStep_ = 0;
		hasComboInput_ = false;
		ChangeState(STATE::PARRY);
		return;
	}

	// 攻撃モーション中にクリックされたら先行入力フラグを立てる（最大3段目まで）
	if (ins.IsTrgMouseLeft() && attackStep_ < 4)
	{
		hasComboInput_ = true;
	}

	// 攻撃アニメーションが終わるまで他の行動を遮断
	// 現在の攻撃アニメーションが終了したタイミングの処理
	if (animController_->IsEnd())
	{
		// 攻撃終了時は判定をオフにする
		weaponManager_->SetAttackEnable(WEAPON_SLOT::RIGHT_HAND, false);

		// 先行入力があり、かつ3段目未満であれば次のコンボへ
		if (hasComboInput_ && attackStep_ < 4)
		{
			attackStep_++;
			ChangeState(STATE::ATTACK); // 次の攻撃アニメーションを再生
		}
		else
		{
			// 先行入力がない、または3段目が終わったらコンボリセットして待機状態へ
			attackStep_ = 0;
			hasComboInput_ = false;
			ChangeState(STATE::IDLE);
		}
	}
}

void Player::UpdateParry(void)
{
	bool isDash = false;
	VECTOR dir = GetInputDirection(isDash);
	// 移動
	if (isDash)
	{
		ChangeState(STATE::RUN);
		return;
	}

	// パリィアニメーションが終わるまで他の行動を遮断
	if (animController_->IsEnd())
	{
		ChangeState(STATE::IDLE);
	}
}

//// 右手の武器の種類によって攻撃アニメーションや判定を変える
	//WEAPON_TYPE rightWeapon = weaponManager_->GetWeaponType(WEAPON_SLOT::RIGHT_HAND);

	//switch (rightWeapon)
	//{
	//case WEAPON_TYPE::SWORD:
	//	// 片手剣攻撃の処理・アニメーション再生
	//	break;

	//case WEAPON_TYPE::GREAT_SWORD:
	//	// 大剣攻撃の処理
	//	break;

	//case WEAPON_TYPE::GUN:
	//	// 射撃処理
	//	break;

	//default:
	//	// 素手攻撃
	//	break;
	//}

void Player::UpdateProcessPost(void)
{
}

void Player::DrawDegug(void)
{
	// 所有しているコライダの描画
	for (const auto& own : ownColliders_)
	{
		own.second->Draw();
	}
	
}

void Player::InitTransform(void)
{
	transform_.scl = PLAYER_DEFAULT_SCALE;
	transform_.quaRot = Quaternion::Identity();
	transform_.quaRotLocal = Quaternion::Identity();
	transform_.quaRotLocal = Quaternion::Euler(PLAYER_DEFAULT_ROT_LOCAL);
	transform_.pos = DEFAULT_POS;
	transform_.localPos = DEFAULT_LOCAL_POS;
	transform_.Update();

	weaponManager_->Init(transform_.modelId);

	// 剣インスタンスを作成して装備
	auto sword = std::make_shared<WeaponSword>();
	weaponManager_->EquipWeapon(
		WEAPON_SLOT::RIGHT_HAND,
		sword,
		swordHandle_, 43, SWORD_SCL,
		SWORD_LOCALPOS, SWORD_LOCALROT
	);

	/*auto shield = std::make_shared<WeaponShield>();
	weaponManager_->EquipWeapon(
		WEAPON_SLOT::LEFT_HAND,
		shield,
		shieldHandle_, 12, SHIELD_SCL,
		SHIELD_LOCALPOS, SHIELD_LOCALROT
	);*/	
}

void Player::InitCollider(void)
{
	// 主に地面との衝突で使用する線分コライダ
	ColliderLine* colLine = new ColliderLine(
		ColliderBase::TAG::PLAYER, &transform_,
		COL_LINE_START_LOCAL_POS, COL_LINE_END_LOCAL_POS);
	ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::LINE), colLine);

	// 主に壁や木などの衝突で仕様するカプセルコライダ
	ColliderCapsule* colCapsule = new ColliderCapsule(
		ColliderBase::TAG::PLAYER, &transform_,
		COL_CAPSULE_TOP_LOCAL_POS, COL_CAPSULE_DOWN_LOCAL_POS,
		COL_CAPSULE_RADIUS);
	ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::CAPSULE), colCapsule);
}

void Player::InitAnimation(void)
{
	animController_ = std::make_shared<AnimationController>(transform_.modelId);

	animController_->Add(static_cast<int>(ANIM_TYPE::IDLE), 30.0f,
		Application::PATH_MODEL + "Charactor/Player/idle.mv1");

	animController_->Add(static_cast<int>(ANIM_TYPE::RUN), 30.0f,
		Application::PATH_MODEL + "Charactor/Player/run.mv1");

	animController_->Add(static_cast<int>(ANIM_TYPE::FAST_RUN), 30.0f,
		Application::PATH_MODEL + "Charactor/Player/Run To Stop.mv1");

	animController_->Add(static_cast<int>(ANIM_TYPE::WALK), 30.0f,
		Application::PATH_MODEL + "Charactor/Player/walk.mv1");

	animController_->Add(static_cast<int>(ANIM_TYPE::SS_PARRY), 45.0f,
		Application::PATH_MODEL + "Charactor/Player/ShortSword/parry.mv1");

	animController_->Add(static_cast<int>(ANIM_TYPE::SS_RUN), 30.0f,
		Application::PATH_MODEL + "Charactor/Player/ShortSword/shortSwordRun.mv1");

	animController_->Add(static_cast<int>(ANIM_TYPE::SS_UNSHEATHED), 30.0f,
		Application::PATH_MODEL + "Charactor/Player/ShortSword/unsheathedSword.mv1");

	animController_->Add(static_cast<int>(ANIM_TYPE::SS_SHEATHING), 30.0f,
		Application::PATH_MODEL + "Charactor/Player/ShortSword/sheathingSword.mv1");

	animController_->Add(static_cast<int>(ANIM_TYPE::SS_ATTACK_1), 45.0f,
		Application::PATH_MODEL + "Charactor/Player/ShortSword/swordSlash_1.mv1");
	animController_->Add(static_cast<int>(ANIM_TYPE::SS_ATTACK_2), 45.0f,
		Application::PATH_MODEL + "Charactor/Player/ShortSword/swordSlash_2.mv1");
	animController_->Add(static_cast<int>(ANIM_TYPE::SS_ATTACK_3), 45.0f,
		Application::PATH_MODEL + "Charactor/Player/ShortSword/swordSlash_3.mv1");
	animController_->Add(static_cast<int>(ANIM_TYPE::SS_ATTACK_4), 45.0f,
		Application::PATH_MODEL + "Charactor/Player/ShortSword/swordSlash_4.mv1");
	animController_->Add(static_cast<int>(ANIM_TYPE::SS_DASHATTACK), 45.0f,
		Application::PATH_MODEL + "Charactor/Player/ShortSword/swordDashAttack2.mv1");

	animController_->Play(static_cast<int>(ANIM_TYPE::IDLE));
}

void Player::InitPost(void)
{
	isParry_ = false;
	ChangeState(STATE::IDLE);
}
