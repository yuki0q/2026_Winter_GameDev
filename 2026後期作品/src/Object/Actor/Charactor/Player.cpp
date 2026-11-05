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
#include <map>
#include <string>
#include <cmath>

namespace
{
	// 武器種別から装備するモデルのリソースを決める
	// 武器を増やす時は ResourceManager::SRC にモデルを登録して、ここに case を足す
	bool GetWeaponModelSrc(WEAPON_TYPE type, ResourceManager::SRC& outSrc)
	{
		switch (type)
		{
		case WEAPON_TYPE::SWORD:
			outSrc = ResourceManager::SRC::MDL_SWORD;
			return true;
		case WEAPON_TYPE::SHIELD:
			outSrc = ResourceManager::SRC::MDL_SHIELD;
			return true;
		default:
			return false;
		}
	}

	// 武器種別から武器クラスを生成
	std::shared_ptr<WeaponBase> CreateWeapon(WEAPON_TYPE type)
	{
		switch (type)
		{
		case WEAPON_TYPE::SWORD:
			return std::make_shared<WeaponSword>();
		case WEAPON_TYPE::SHIELD:
			return std::make_shared<WeaponShield>();
		default:
			return nullptr;
		}
	}

	// CSV上のスロット名から WEAPON_SLOT へ
	WEAPON_SLOT SlotFromName(const std::string& name)
	{
		if (name == "LEFT_HAND") return WEAPON_SLOT::LEFT_HAND;
		if (name == "BACK")      return WEAPON_SLOT::BACK;
		return WEAPON_SLOT::RIGHT_HAND;
	}

	// 武器モデルの複製を読み込む（モデルが登録されていなければ -1）
	int LoadWeaponModel(WEAPON_TYPE type)
	{
		ResourceManager::SRC src;
		if (!GetWeaponModelSrc(type, src))
		{
			return -1;
		}
		return ResourceManager::GetInstance().LoadModelDuplicate(src);
	}
}

Player::Player(void)
	:
	mainWeaponType_(WEAPON_TYPE::NONE),
	subWeaponType_(WEAPON_TYPE::NONE),
	mainHandle_(-1),
	subHandle_(-1),
	isParry_(false),
	currentState_(),
	isDashEnable_(true),
	attackStep_(),
	parryedBackSpeed_(),
	hasComboInput_(false),
	comboTimer_(0.0f),
	lastAttackStep_(0),
	isWarning_(false)
{
	// CSVからパラメータを取得（Init()より前に確定させる）
	param_ = PlayerDataTable::GetParam();
	mainWeaponType_ = NameToWeaponType(param_.mainWeapon);
	subWeaponType_ = NameToWeaponType(param_.subWeapon);

	// メイン武器の種類に応じた動きのパラメータ
	style_ = PlayerDataTable::GetStyle(param_.mainWeapon);
}

Player::~Player(void)
{
}

void Player::Draw(void)
{
	if (isDead_) return;

	CharactorBase::Draw();

	weaponManager_->Draw();

	if (!isDebug_)return;
	DrawDegug();
}

void Player::Release(void)
{
	CharactorBase::Release();
	weaponManager_->Release();

	if (mainHandle_ != -1) { MV1DeleteModel(mainHandle_); }
	if (subHandle_ != -1) { MV1DeleteModel(subHandle_); }
}

void Player::OnDamage(int damage ,VECTOR attackedPos)
{
	if (hp_ <= 0 || (currentState_ == STATE::DAMAGE) || (currentState_ == STATE::PARRYATTACK)
		|| (currentState_ == STATE::AVOID)) return;

	hp_ -= damage;
	if (hp_ <= 0)
	{
		hp_ = 0;
		ChangeState(STATE::DIE);
	}
	else
	{
		// ダメージを受けたときにDAMAGEステートに移行する
		ChangeState(STATE::DAMAGE);

		KnockBack(attackedPos, param_.knockbackPower, param_.knockbackTime);
	}
}

void Player::InDamage(int damage)
{
	if (hp_ <= 0 || (currentState_ == STATE::DAMAGE) || (currentState_ == STATE::PARRYATTACK)
		|| (currentState_ == STATE::AVOID)) return;

	hp_ -= damage;
	if (hp_ <= 0)
	{
		hp_ = 0;
		ChangeState(STATE::DIE);
	}
	else
	{
		// ダメージを受けたときにDAMAGEステートに移行する
		ChangeState(STATE::DAMAGE);
	}
}

void Player::AddHP(int addDamage)
{
	if (hp_ <= 0)return;
	hp_ += addDamage;

	if (hp_ >= hpMax_)
	{
		hp_ = hpMax_;
		ChangeState(STATE::IDLE);
	}
}

void Player::SetPosition(const VECTOR& pos)
{
	transform_.pos = pos;
	transform_.Update();
}

void Player::ClearParry(void)
{
	isParry_ = true;
}

void Player::LookAt(const VECTOR& targetPos)
{
	VECTOR dir = VSub(targetPos, transform_.pos);
	dir.y = 0.0f; // Y軸方向の傾きを無視して水平方向のみにする

	if (VSize(dir) > 0.001f)
	{
		dir = AsoUtility::VNormalize(dir);

		// 方向ベクトルからクォータニオンの回転を作成して反映
		float angle = atan2f(dir.x, dir.z);
		transform_.quaRot = Quaternion::AngleAxis(angle, AsoUtility::AXIS_Y);
		transform_.Update();
	}
}

void Player::InitLoad(void)
{
	transform_.SetModel(resMng_.LoadModelDuplicate(
		ResourceManager::SRC::MDL_PLAYER));
	
	weaponManager_ = std::make_shared<WeaponManager>();

	// 武器モデル（種類は player_param.csv の mainWeapon / subWeapon）
	mainHandle_ = LoadWeaponModel(mainWeaponType_);
	subHandle_ = LoadWeaponModel(subWeaponType_);
}

void Player::UpdateProcess(void)
{
	if (isDead_) return;

	// コンボ猶予時間の経過
	UpdateComboTimer();

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
	case STATE::AVOID:
		UpdateAvoid();    
		break;
	case STATE::ATTACK: 
		UpdateAttack();
		break;
	case STATE::PARRY: 
		UpdateParry();  
		break;
	case STATE::PARRYED: 
		UpdateParryed();  
		break;
	case STATE::PARRYATTACK:
		UpdateParryAttack();
		break;
	case STATE::DAMAGE: 
		UpdateDamage();  
		break;
	case STATE::DIE: 
		UpdateDie();  
		break;
	case STATE::END: 
		UpdateEnd();  
		break;
	}
}

float Player::GetAttackDamageRate(void) const
{
	// 現在のモーション名（player_anim.csv の name と同じ名前で player_attack.csv を引く）
	std::string motion;

	switch (currentState_)
	{
	case STATE::ATTACK:
		if (attackStep_ == 0)
		{
			// ダッシュ攻撃
			motion = "DASH_ATTACK";
		}
		else
		{
			// コンボ段（ATTACK_1, ATTACK_2, ...）
			motion = "ATTACK_" + std::to_string(attackStep_);
		}
		break;

	case STATE::PARRYATTACK:
		motion = "PARRY_ATTACK";
		break;

	default:
		// 攻撃中以外は等倍
		return 1.0f;
	}

	return PlayerDataTable::GetAttackRate(param_.mainWeapon, motion);
}

int Player::CalcAttackDamage(int baseDamage) const
{
	int damage = static_cast<int>(std::lround(
		static_cast<float>(baseDamage) * GetAttackDamageRate()));

	// 倍率を小さくしすぎても、ヒットした攻撃のダメージが0にならないようにする
	return damage < 1 ? 1 : damage;
}

void Player::UpdateComboTimer(void)
{
	if (comboTimer_ <= 0.0f)
	{
		return;
	}

	comboTimer_ -= scnMng_.GetDeltaTime();

	// 猶予切れでコンボのカウントをリセット
	if (comboTimer_ <= 0.0f)
	{
		ResetCombo();
	}
}

int Player::GetStartAttackStep(void) const
{
	// 猶予中で、まだ続きの段がある場合は、前回の次の段から再開
	if (comboTimer_ > 0.0f &&
		lastAttackStep_ >= 1 && lastAttackStep_ < style_.comboMax)
	{
		return lastAttackStep_ + 1;
	}

	return 1;
}

void Player::ResetCombo(void)
{
	comboTimer_ = 0.0f;
	lastAttackStep_ = 0;
}

void Player::ChangeState(STATE state)
{
	currentState_ = state;

	switch (currentState_)
	{
	case STATE::IDLE:
		animController_->Play(static_cast<int>(ANIM_TYPE::IDLE), true);
		moveDir_ = Quaternion::PosAxis(transform_.quaRot, AsoUtility::DIR_F);
		break;
	case STATE::WALK:
		animController_->Play(static_cast<int>(ANIM_TYPE::WALK), true);
		break;
	case STATE::RUN:
		animController_->Play(static_cast<int>(ANIM_TYPE::WEAPON_RUN), true);
		break;
	case STATE::AVOID:
		animController_->Play(static_cast<int>(ANIM_TYPE::AVOID), false);
		break;
	case STATE::ATTACK:
		// 猶予からの再開でも新規でも、攻撃中は猶予を使わない（終了時に改めて設定する）
		comboTimer_ = 0.0f;
		// 攻撃開始時にダッシュ入力をリセット
		isDashEnable_ = false; 
		// 先行入力フラグをクリア
		hasComboInput_ = false; 

		// 攻撃ステップに応じたアニメーションの再生
		if (attackStep_ == 0)
		{
			animController_->Play(static_cast<int>(ANIM_TYPE::DASH_ATTACK), false);
		}
		if (attackStep_ == 1)
		{
			animController_->Play(static_cast<int>(ANIM_TYPE::ATTACK_1), false);
		}
		else if (attackStep_ == 2)
		{
			animController_->Play(static_cast<int>(ANIM_TYPE::ATTACK_2), false);
		}
		else if (attackStep_ == 3)
		{
			animController_->Play(static_cast<int>(ANIM_TYPE::ATTACK_3), false);
		}
		else if (attackStep_ == 4)
		{
			animController_->Play(static_cast<int>(ANIM_TYPE::ATTACK_4), false);
		}

		break;
	case STATE::PARRY:
		animController_->Play(static_cast<int>(ANIM_TYPE::PARRY), false);
		isDashEnable_ = false;
		break;
	case STATE::PARRYED:
		animController_->Play(static_cast<int>(ANIM_TYPE::PARRYED), false);
		parryedBackSpeed_ = style_.parryedBackSpeed;
		moveDir_ = Quaternion::PosAxis(transform_.quaRot, AsoUtility::DIR_F);
		isDashEnable_ = false;
		break;
	case STATE::PARRYATTACK:
		animController_->Play(static_cast<int>(ANIM_TYPE::PARRYATTACK), false);
		isDashEnable_ = false;
		break;
	case STATE::DAMAGE:
		animController_->Play(static_cast<int>(ANIM_TYPE::DAMAGE), false);
		// 攻撃終了時は判定をオフにする
		weaponManager_->SetAttackEnable(WEAPON_SLOT::RIGHT_HAND, false);
		isDashEnable_ = false;
		break;
	case STATE::DIE:
		animController_->Play(static_cast<int>(ANIM_TYPE::DIE), false);
		// 攻撃終了時は判定をオフにする
		weaponManager_->SetAttackEnable(WEAPON_SLOT::RIGHT_HAND, false);
		break;
	case STATE::END:
		break;
	}

	isParry_ = false;
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

		if (ins.IsNew(KEY_INPUT_RETURN)) { transform_.pos = param_.startPos; }
	}
	else
	{
		InputManager::JOYPAD_IN_STATE padState = ins.GetJPadInputState(InputManager::JOYPAD_NO::PAD1);
		dir = ins.GetDirectionXZAKey(padState.AKeyLX, padState.AKeyLY);

	}

	bool KeyDash = ins.IsActionNew(InputManager::ACTION::AVOID_DASH);

	// ダッシュ入力のリセット判定
	if (!KeyDash)
	{
		// キーが一度でも離されたらダッシュ入力を再度有効化
		isDashEnable_ = true;
	}

	// フラグが true の場合のみダッシュ判定を有効にする
	if (isDashEnable_ && KeyDash)
	{
		IsDash = true;
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

	bool IsAttack = ins.IsActionTrgDown(InputManager::ACTION::ATTACK);
	bool IsParry = ins.IsActionTrgDown(InputManager::ACTION::PARRY);

	// 攻撃優先（クリック）
	if (IsAttack)
	{
		attackStep_ = GetStartAttackStep(); // 通常は1段目、猶予中は続きの段から
		ChangeState(STATE::ATTACK);
		return;
	}

	// パリィ優先（Spaceキー）
	if (IsParry)
	{
		ChangeState(STATE::PARRY);
		return;
	}

	// 移動
	bool isDash = false;
	VECTOR dir = GetInputDirection(isDash);
	if (AsoUtility::EqualsVZero(dir))return;
	if (isDash)
	{
		// 敵の攻撃前状態による警告を受けている場合は、ダッシュではなく回避を発動！
		if (isWarning_)
		{
			ChangeState(STATE::AVOID);
			return;
		}

		ChangeState(STATE::RUN);
		return;
	}
	else if(!isDash)
	{
		ChangeState(STATE::WALK);
		return;
	}
}

void Player::UpdateWalk(void)
{
	auto& ins = InputManager::GetInstance();

	bool IsAttack = ins.IsActionTrgDown(InputManager::ACTION::ATTACK);
	bool IsParry = ins.IsActionTrgDown(InputManager::ACTION::PARRY);

	if (IsAttack)
	{
		attackStep_ = GetStartAttackStep(); // 通常は1段目、猶予中は続きの段から
		ChangeState(STATE::ATTACK);
		return;
	}

	if (IsParry)
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
		// 敵の攻撃前状態による警告を受けている場合は、ダッシュではなく回避を発動！
		if (isWarning_)
		{
			ChangeState(STATE::AVOID);
			return;
		}

		ChangeState(STATE::RUN);
		return;
	}

	MoveDirection(dir, style_.walkSpeed);
}

void Player::UpdateRun(void)
{
	auto& ins = InputManager::GetInstance();

	bool IsAttack = ins.IsActionTrgDown(InputManager::ACTION::ATTACK);
	bool IsParry = ins.IsActionTrgDown(InputManager::ACTION::PARRY);

	if (IsAttack)
	{
		attackStep_ = 0; // ダッシュ攻撃から開始
		ChangeState(STATE::ATTACK);
		return;
	}

	if (IsParry)
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

	MoveDirection(dir, style_.dashSpeed);
}

void Player::UpdateAvoid(void)
{
	auto& ins = InputManager::GetInstance();

	bool IsAttack = ins.IsActionTrgDown(InputManager::ACTION::ATTACK);
	bool IsParry = ins.IsActionTrgDown(InputManager::ACTION::PARRY);

	// 攻撃優先（クリック）
	if (IsAttack)
	{
		attackStep_ = GetStartAttackStep(); // 通常は1段目、猶予中は続きの段から
		ChangeState(STATE::ATTACK);
		return;
	}

	// パリィ優先（Spaceキー）
	if (IsParry)
	{
		ChangeState(STATE::PARRY);
		return;
	}
	
	bool isDash = false;
	VECTOR dir = GetInputDirection(isDash);
	MoveDirection(dir, style_.dashSpeed);

	if (!animController_->IsEnd())return;
	
	if (AsoUtility::EqualsVZero(dir))
	{
		ChangeState(STATE::IDLE);
		return;
	}

	if (!AsoUtility::EqualsVZero(dir))
	{
		ChangeState(isDash ? STATE::RUN : STATE::WALK);
		return;
	}
}

void Player::UpdateAttack(void)
{	
	// 攻撃中は武器の当たり判定を有効化
	weaponManager_->SetAttackEnable(WEAPON_SLOT::RIGHT_HAND, true);

	if (attackStep_ == 0 || attackStep_ == 4)
	{
		VECTOR forward = Quaternion::PosAxis(transform_.quaRot, AsoUtility::DIR_F);
		VECTOR attackMove = VScale(forward, style_.attackMove);
		transform_.pos = VAdd(transform_.pos, attackMove);
	}

	auto& ins = InputManager::GetInstance();

	bool IsAttack = ins.IsActionTrgDown(InputManager::ACTION::ATTACK);
	bool IsParry = ins.IsActionTrgDown(InputManager::ACTION::PARRY);

	bool isDash = false;
	VECTOR dir = GetInputDirection(isDash);

	// ダッシュで攻撃をキャンセル
	if (isDash)
	{
		attackStep_ = 0;
		hasComboInput_ = false;
		ResetCombo();
		weaponManager_->SetAttackEnable(WEAPON_SLOT::RIGHT_HAND, false);

		// 敵の攻撃前状態による警告を受けている場合は、ダッシュではなく回避を発動！
		if (isWarning_)
		{
			ChangeState(STATE::AVOID);
			return;
		}

		ChangeState(STATE::RUN);
		return;
	}

	// 攻撃中もパリィが出せるように
	if (IsParry)
	{
		attackStep_ = 0;
		hasComboInput_ = false;
		ResetCombo();
		weaponManager_->SetAttackEnable(WEAPON_SLOT::RIGHT_HAND, false);
		ChangeState(STATE::PARRY);
		return;
	}

	// 攻撃モーション中にクリックされたら先行入力フラグを立てる
	if (IsAttack && attackStep_ < style_.comboMax)
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
		if (hasComboInput_ && attackStep_ < style_.comboMax)
		{
			attackStep_++;
			ChangeState(STATE::ATTACK); // 次の攻撃アニメーションを再生
		}
		else
		{
			// 先行入力がなくても、最終段でなければ猶予時間の間はコンボを保持する
			// （猶予中に攻撃すると続きの段から始まり、切れると UpdateComboTimer でリセットされる）
			if (attackStep_ >= 1 && attackStep_ < style_.comboMax)
			{
				lastAttackStep_ = attackStep_;
				comboTimer_ = style_.comboGraceTime;
			}
			else
			{
				// ダッシュ攻撃や最終段まで出し切った場合は、猶予なしでリセット
				ResetCombo();
			}

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

	if (isParry_)
	{
		ChangeState(STATE::PARRYED);
	}

	// 移動
	if (isDash)
	{
		// 敵の攻撃前状態による警告を受けている場合は、ダッシュではなく回避を発動！
		if (isWarning_)
		{
			ChangeState(STATE::AVOID);
			return;
		}

		ChangeState(STATE::RUN);
		return;
	}

	// パリィアニメーションが終わるまで他の行動を遮断
	if (animController_->IsEnd())
	{
		ChangeState(STATE::IDLE);
	}
}

void Player::UpdateParryed(void)
{
	// パリィ後の後退処理
	if (parryedBackSpeed_ > 0.0f)
	{
		// プレイヤーが向いている正面の「逆方向（背面）」を計算
		VECTOR forward = Quaternion::PosAxis(transform_.quaRot, AsoUtility::DIR_F);
		VECTOR backMove = VScale(forward, -parryedBackSpeed_);

		// 座標に加算
		transform_.pos = VAdd(transform_.pos, backMove);

		// 勢いを少しずつ減衰させる（摩擦）
		parryedBackSpeed_ *= 0.8f;

		if (parryedBackSpeed_ < 0.1f)
		{
			parryedBackSpeed_ = 0.0f;
		}
	}

	auto& ins = InputManager::GetInstance();

	bool IsAttack = ins.IsActionTrgDown(InputManager::ACTION::ATTACK);

	// 攻撃モーション中にクリックされたら先行入力フラグを立てる（最大3段目まで）
	if (IsAttack)
	{
		hasComboInput_ = true;
	}

	if (animController_->IsEnd())
	{
		ChangeState(STATE::IDLE);
	}

	if (hasComboInput_)
	{
		hasComboInput_ = false;
		ChangeState(STATE::PARRYATTACK); // 次の攻撃アニメーションを再生
	}
}

void Player::UpdateParryAttack(void)
{
	// 攻撃中は武器の当たり判定を有効化
	weaponManager_->SetAttackEnable(WEAPON_SLOT::RIGHT_HAND, true);

	VECTOR forward = Quaternion::PosAxis(transform_.quaRot, AsoUtility::DIR_F);
	VECTOR attackMove = VScale(forward, style_.parryAttackMove);
	transform_.pos = VAdd(transform_.pos, attackMove);

	// パリィアニメーションが終わるまで他の行動を遮断
	if (animController_->IsEnd())
	{
		// 攻撃終了時は判定をオフにする
		weaponManager_->SetAttackEnable(WEAPON_SLOT::RIGHT_HAND, false);

		ChangeState(STATE::IDLE);
	}
}

void Player::UpdateDamage(void)
{
	if (animController_->IsEnd())
	{
		ChangeState(STATE::IDLE);
	}
}

void Player::UpdateDie(void)
{
	if (animController_->IsEnd())
	{
		ChangeState(STATE::END);
	}
}

void Player::UpdateEnd(void)
{
	isDead_ = true;
}

void Player::DelayRotate(void)
{
	// 攻撃中、またはパリィ攻撃中などは、LookAtで向いた方向を維持するため自動回転をさせない
	if (currentState_ == STATE::ATTACK ||
		currentState_ == STATE::PARRYATTACK ||
		currentState_ == STATE::DAMAGE)
	{
		return;
	}

	CharactorBase::DelayRotate();
}

void Player::UpdateProcessPost(void)
{
	if (isDead_) return;

	weaponManager_->Update();
}

int Player::GetAttackDamage(void) const
{
	if (!weaponManager_)
	{
		return 10;
	}

	const auto& sword = weaponManager_->GetWeapon(WEAPON_SLOT::RIGHT_HAND);
	return sword ? sword->GetDamage() : 10;
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
	transform_.scl = { param_.scale, param_.scale, param_.scale };
	transform_.quaRot = Quaternion::Identity();
	transform_.quaRotLocal = Quaternion::Identity();
	transform_.quaRotLocal = Quaternion::Euler(PLAYER_DEFAULT_ROT_LOCAL);
	transform_.pos = param_.startPos;
	transform_.localPos = param_.localPos;
	transform_.Update();

	weaponManager_->Init(transform_.modelId);

	// メイン武器・サブ武器を装備（装着位置などは weapon_param.csv）
	EquipFromTable(mainWeaponType_, mainHandle_);
	EquipFromTable(subWeaponType_, subHandle_);
}

void Player::EquipFromTable(WEAPON_TYPE type, int modelHandle)
{
	if (type == WEAPON_TYPE::NONE || modelHandle == -1)
	{
		return;
	}

	auto weapon = CreateWeapon(type);
	if (!weapon)
	{
		return;
	}

	const WeaponParam wp = PlayerDataTable::GetWeaponParam(WeaponTypeToName(type));

	weaponManager_->EquipWeapon(
		SlotFromName(wp.slot),
		weapon,
		modelHandle, wp.attachFrame,
		VGet(wp.scale, wp.scale, wp.scale),
		wp.localPos, wp.localRot
	);
}

void Player::InitCollider(void)
{
	// 主に地面との衝突で使用する線分コライダ
	ColliderLine* colLine = new ColliderLine(
		ColliderBase::TAG::PLAYER, &transform_,
		VGet(0.0f, param_.lineTopY, 0.0f), AsoUtility::VECTOR_ZERO);
	ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::LINE), colLine);

	// 主に壁や木などの衝突で仕様するカプセルコライダ
	ColliderCapsule* colCapsule = new ColliderCapsule(
		ColliderBase::TAG::PLAYER, &transform_,
		VGet(0.0f, param_.capsuleTopY, 0.0f), VGet(0.0f, param_.capsuleDownY, 0.0f),
		param_.capsuleRadius);
	ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::CAPSULE), colCapsule);
}

void Player::InitAnimation(void)
{
	animController_ = std::make_shared<AnimationController>(transform_.modelId);

	// CSV上の名前と ANIM_TYPE の対応表（新しいアニメを増やす時はここに追記）
	static const std::map<std::string, ANIM_TYPE> NAME_TO_ANIM = {
		{ "IDLE",         ANIM_TYPE::IDLE },
		{ "WALK",         ANIM_TYPE::WALK },
		{ "RUN",          ANIM_TYPE::RUN },
		{ "WEAPON_RUN",   ANIM_TYPE::WEAPON_RUN },
		{ "FAST_RUN",     ANIM_TYPE::FAST_RUN },
		{ "JUMP",         ANIM_TYPE::JUMP },
		{ "ATTACK_1",     ANIM_TYPE::ATTACK_1 },
		{ "ATTACK_2",     ANIM_TYPE::ATTACK_2 },
		{ "ATTACK_3",     ANIM_TYPE::ATTACK_3 },
		{ "ATTACK_4",     ANIM_TYPE::ATTACK_4 },
		{ "DASH_ATTACK",  ANIM_TYPE::DASH_ATTACK },
		{ "AVOID",        ANIM_TYPE::AVOID },
		{ "PARRY",        ANIM_TYPE::PARRY },
		{ "PARRYED",      ANIM_TYPE::PARRYED },
		{ "PARRY_ATTACK", ANIM_TYPE::PARRYATTACK },
		{ "SHEATHING",    ANIM_TYPE::SHEATHING },
		{ "UNSHEATHED",   ANIM_TYPE::UNSHEATHED },
		{ "DAMAGE",       ANIM_TYPE::DAMAGE },
		{ "DIE",          ANIM_TYPE::DIE },
	};

	// 共通アニメ + メイン武器専用アニメ（同名は武器専用が優先）
	for (const auto& anim : PlayerDataTable::GetAnims(param_.mainWeapon))
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

	animController_->Play(static_cast<int>(ANIM_TYPE::IDLE));
}

void Player::InitPost(void)
{
	hp_ = hpMax_ = param_.hp;;
	isDead_ = false;
	isParry_ = false;
	ChangeState(STATE::IDLE);
}