#include <DxLib.h>
#include "InputManager.h"

InputManager* InputManager::instance_ = nullptr;

void InputManager::CreateInstance(void)
{
	if (instance_ == nullptr)
	{
		instance_ = new InputManager();
	}
	instance_->Init();
}

InputManager& InputManager::GetInstance(void)
{
	if (instance_ == nullptr)
	{
		InputManager::CreateInstance();
	}
	return *instance_;
}

void InputManager::Init(void)
{

	// ゲームで使用したいキーを、
	// 事前にここで登録しておいてください
	InputManager::GetInstance().Add(KEY_INPUT_SPACE);
	InputManager::GetInstance().Add(KEY_INPUT_ESCAPE);
	InputManager::GetInstance().Add(KEY_INPUT_RETURN);
	InputManager::GetInstance().Add(KEY_INPUT_N);
	InputManager::GetInstance().Add(KEY_INPUT_Z);
	InputManager::GetInstance().Add(KEY_INPUT_W);
	InputManager::GetInstance().Add(KEY_INPUT_A);
	InputManager::GetInstance().Add(KEY_INPUT_S);
	InputManager::GetInstance().Add(KEY_INPUT_D);
	InputManager::GetInstance().Add(KEY_INPUT_Q);
	InputManager::GetInstance().Add(KEY_INPUT_E);
	InputManager::GetInstance().Add(KEY_INPUT_F);
	InputManager::GetInstance().Add(KEY_INPUT_B);
	InputManager::GetInstance().Add(KEY_INPUT_UP);
	InputManager::GetInstance().Add(KEY_INPUT_DOWN);
	InputManager::GetInstance().Add(KEY_INPUT_RIGHT);
	InputManager::GetInstance().Add(KEY_INPUT_LEFT);
	InputManager::GetInstance().Add(KEY_INPUT_LSHIFT);
	InputManager::GetInstance().Add(KEY_INPUT_RSHIFT);
	InputManager::GetInstance().Add(KEY_INPUT_NUMPAD1);
	InputManager::GetInstance().Add(KEY_INPUT_1);
	InputManager::GetInstance().Add(KEY_INPUT_BACKSLASH);
	InputManager::GetInstance().Add(KEY_INPUT_R);
	InputManager::GetInstance().Add(KEY_INPUT_LALT);
	InputManager::GetInstance().Add(KEY_INPUT_X);
	InputManager::GetInstance().Add(KEY_INPUT_C);

	InputManager::MouseInfo info;

	// 左クリック
	info = InputManager::MouseInfo();
	info.key = MOUSE_INPUT_LEFT;
	info.keyOld = false;
	info.keyNew = false;
	info.keyTrgDown = false;
	info.keyTrgUp = false;
	mouseInfos_.emplace(info.key, info);

	// 右クリック
	info = InputManager::MouseInfo();
	info.key = MOUSE_INPUT_RIGHT;
	info.keyOld = false;
	info.keyNew = false;
	info.keyTrgDown = false;
	info.keyTrgUp = false;
	mouseInfos_.emplace(info.key, info);

	// デフォルトのキーコンフィグ設定
	// 攻撃 = 左クリック または パッドの Y/上ボタン
	SetBinding(ACTION::ATTACK, -1, MOUSE_INPUT_LEFT, JOYPAD_BTN::TOP);

	// パリィ/シーン遷移 = SPACEキー または パッドの B/右ボタン
	SetBinding(ACTION::PARRY, KEY_INPUT_SPACE, -1, JOYPAD_BTN::RIGHT);

	// 回避/ダッシュ = LSHIFTキー または パッドの A/下ボタン
	SetBinding(ACTION::AVOID_DASH, KEY_INPUT_LSHIFT, -1, JOYPAD_BTN::DOWN);

	// アイテムを拾う = Fキー または パッドの X/左ボタン
	SetBinding(ACTION::PICK_UP, KEY_INPUT_F, -1, JOYPAD_BTN::LEFT);

	// バッグを開く = Bキー または パッドの X/左ボタン + LB
	SetBinding(ACTION::OPEN_BAG, KEY_INPUT_B, -1, JOYPAD_BTN::LB, JOYPAD_BTN::LEFT);

	// スキル = Eキー または パッドの Y/上ボタン + LB
	SetBinding(ACTION::SKILL, KEY_INPUT_E, -1, JOYPAD_BTN::LB, JOYPAD_BTN::TOP);

	// 必殺技 = Qキー または パッドのLB + RB
	SetBinding(ACTION::ULTIMET, KEY_INPUT_Q, -1, JOYPAD_BTN::LB, JOYPAD_BTN::RB);

	// ポーズ/閉じる = ECSキー または パッドのSTART
	SetBinding(ACTION::PAUSE, KEY_INPUT_ESCAPE, -1, JOYPAD_BTN::START);
}

void InputManager::SetBinding(ACTION action, int kbKey, int mouseBtn, JOYPAD_BTN padBtn1, JOYPAD_BTN padBtn2)
{
	bindings_[action] = { kbKey, mouseBtn, padBtn1, padBtn2 };
}

const InputManager::Binding& InputManager::GetBinding(ACTION action) const
{
	auto it = bindings_.find(action);
	if (it != bindings_.end()) {
		return it->second;
	}
	static Binding empty{ -1, -1, JOYPAD_BTN::MAX };
	return empty;
}

bool InputManager::IsActionNew(ACTION action) const
{
	auto it = bindings_.find(action);
	if (it == bindings_.end()) return false;

	const auto& bind = it->second;

	// キーボード判定
	if (bind.keyBoardKey != -1 && IsNew(bind.keyBoardKey)) {
		return true;
	}
	// マウス判定
	if (bind.mouseButton != -1 && FindMouse(bind.mouseButton).keyNew) {
		return true;
	}
	// パッド判定 (PAD1を対象とする例)
	if (bind.padBtn1 != JOYPAD_BTN::MAX && IsPadBtnNew(JOYPAD_NO::PAD1, bind.padBtn1)) {
		return true;
	}

	return false;
}

bool InputManager::IsActionTrgDown(ACTION action) const
{
	auto it = bindings_.find(action);
	if (it == bindings_.end()) return false;

	const auto& bind = it->second;

	// キーボード判定
	if (bind.keyBoardKey != -1 && IsTrgDown(bind.keyBoardKey)) {
		return true;
	}
	// マウス判定
	if (bind.mouseButton != -1 && FindMouse(bind.mouseButton).keyTrgDown) {
		return true;
	}
	// パッド判定 (PAD1を対象とする例)
	if (bind.padBtn1 != JOYPAD_BTN::MAX)
	{
		if (bind.padBtn2 != JOYPAD_BTN::MAX)
		{
			// ▼ 同時押しの判定 ▼
			// IsPadBtnNew は「現在押されているか（ホールド）」、IsPadBtnTrgDown は「今押された瞬間か」と仮定しています。
			// ※お使いの関数名に合わせて微調整してください。

			bool btn1Trg = IsPadBtnTrgDown(JOYPAD_NO::PAD1, bind.padBtn1);
			bool btn1On = IsPadBtnNew(JOYPAD_NO::PAD1, bind.padBtn1);

			bool btn2Trg = IsPadBtnTrgDown(JOYPAD_NO::PAD1, bind.padBtn2);
			bool btn2On = IsPadBtnNew(JOYPAD_NO::PAD1, bind.padBtn2);

			// 「Btn2を押しながら Btn1をポンと押した」 または 「Btn1を押しながら Btn2をポンと押した」
			if ((btn1Trg && btn2On) || (btn2Trg && btn1On))
			{
				return true;
			}
		}
		else
		{
			// ▼ 単押しの判定 ▼
			if (IsPadBtnTrgDown(JOYPAD_NO::PAD1, bind.padBtn1))
			{
				return true;
			}
		}
	}

	return false;
}

bool InputManager::IsActionTrgUp(ACTION action) const
{
	auto it = bindings_.find(action);
	if (it == bindings_.end()) return false;

	const auto& bind = it->second;

	// キーボード判定
	if (bind.keyBoardKey != -1 && IsTrgUp(bind.keyBoardKey)) {
		return true;
	}
	// マウス判定
	if (bind.mouseButton != -1 && FindMouse(bind.mouseButton).keyTrgUp) {
		return true;
	}
	// パッド判定 (PAD1を対象とする例)
	if (bind.padBtn1 != JOYPAD_BTN::MAX && IsPadBtnTrgUp(JOYPAD_NO::PAD1, bind.padBtn1)) {
		return true;
	}

	return false;
}

void InputManager::Update(void)
{

	// キーボード検知
	for (auto& p : keyInfos_)
	{
		p.second.keyOld = p.second.keyNew;
		p.second.keyNew = CheckHitKey(p.second.key);
		p.second.keyTrgDown = p.second.keyNew && !p.second.keyOld;
		p.second.keyTrgUp = !p.second.keyNew && p.second.keyOld;
	}

	// マウス検知
	mouseInput_ = GetMouseInput();
	Vector2 prevMousePos = mousePos_;
	GetMousePoint(&mousePos_.x, &mousePos_.y);

	// マウスホイールの回転量を取得
	mouseWheel_ = GetMouseWheelRotVol();

	// 前フレームからの移動量を計算
	mouseDelta_.x = mousePos_.x - prevMousePos.x;
	mouseDelta_.y = mousePos_.y - prevMousePos.y;

	for (auto& p : mouseInfos_)
	{
		p.second.keyOld = p.second.keyNew;
		p.second.keyNew = mouseInput_ == p.second.key;
		p.second.keyTrgDown = p.second.keyNew && !p.second.keyOld;
		p.second.keyTrgUp = !p.second.keyNew && p.second.keyOld;
	}

	// パッド情報
	SetJPadInState(JOYPAD_NO::KEY_PAD1);
	SetJPadInState(JOYPAD_NO::PAD1);
	SetJPadInState(JOYPAD_NO::PAD2);
	SetJPadInState(JOYPAD_NO::PAD3);
	SetJPadInState(JOYPAD_NO::PAD4);

}

void InputManager::Destroy(void)
{
	// インスタンスのメモリ解放
	delete instance_;
}

void InputManager::Add(int key)
{
	InputManager::Info info = InputManager::Info();
	info.key = key;
	info.keyOld = false;
	info.keyNew = false;
	info.keyTrgDown = false;
	info.keyTrgUp = false;
	keyInfos_.emplace(key, info);
}

void InputManager::Clear(void)
{
	keyInfos_.clear();
}

bool InputManager::IsNew(int key) const
{
	return Find(key).keyNew;
}

bool InputManager::IsTrgDown(int key) const
{
	return Find(key).keyTrgDown;
}

bool InputManager::IsTrgUp(int key) const
{
	return Find(key).keyTrgUp;
}

Vector2 InputManager::GetMousePos(void) const
{
	return mousePos_;
}

int InputManager::GetMouse(void) const
{
	return mouseInput_;
}

bool InputManager::IsClickMouseLeft(void) const
{
	return mouseInput_ == MOUSE_INPUT_LEFT;
}

bool InputManager::IsClickMouseRight(void) const
{
	return mouseInput_ == MOUSE_INPUT_RIGHT;
}

bool InputManager::IsTrgMouseLeft(void) const
{
	return FindMouse(MOUSE_INPUT_LEFT).keyTrgDown;
}

bool InputManager::IsTrgMouseRight(void) const
{
	return FindMouse(MOUSE_INPUT_RIGHT).keyTrgDown;
}

int InputManager::GetMouseWheel(void) const
{
	return mouseWheel_;
}

InputManager::InputManager(void)
	:
	joyDInState_(),
	joyXInState_(),
	infoEmpty_(),
	mouseWheel_(),
	padInfos_(),
	mouseInfoEmpty_()
{
	mouseInput_ = -1;
}

const InputManager::Info& InputManager::Find(int key) const
{

	auto it = keyInfos_.find(key);
	if (it != keyInfos_.end())
	{
		return it->second;
	}

	return infoEmpty_;

}

const InputManager::MouseInfo& InputManager::FindMouse(int key) const
{
	auto it = mouseInfos_.find(key);
	if (it != mouseInfos_.end())
	{
		return it->second;
	}

	return mouseInfoEmpty_;
}

InputManager::JOYPAD_TYPE InputManager::GetJPadType(JOYPAD_NO no)
{
	return static_cast<InputManager::JOYPAD_TYPE>(GetJoypadType(static_cast<int>(no)));
}

DINPUT_JOYSTATE InputManager::GetJPadDInputState(JOYPAD_NO no)
{
	// コントローラ情報
	GetJoypadDirectInputState(static_cast<int>(no), &joyDInState_);
	return joyDInState_;
}

XINPUT_STATE InputManager::GetJPadXInputState(JOYPAD_NO no)
{
	// コントローラ情報
	GetJoypadXInputState(static_cast<int>(no), &joyXInState_);
	return joyXInState_;
}

void InputManager::SetJPadInState(JOYPAD_NO jpNo)
{

	int no = static_cast<int>(jpNo);
	auto stateNew = GetJPadInputState(jpNo);
	auto& stateNow = padInfos_[no];

	int max = static_cast<int>(JOYPAD_BTN::MAX);
	for (int i = 0; i < max; i++)
	{

		stateNow.ButtonsOld[i] = stateNow.ButtonsNew[i];
		stateNow.ButtonsNew[i] = stateNew.ButtonsNew[i];

		stateNow.IsOld[i] = stateNow.IsNew[i];
		//stateNow.IsNew[i] = stateNow.ButtonsNew[i] == 128 || stateNow.ButtonsNew[i] == 255;
		stateNow.IsNew[i] = stateNow.ButtonsNew[i] > 0;

		stateNow.IsTrgDown[i] = stateNow.IsNew[i] && !stateNow.IsOld[i];
		stateNow.IsTrgUp[i] = !stateNow.IsNew[i] && stateNow.IsOld[i];


		stateNow.AKeyLX = stateNew.AKeyLX;
		stateNow.AKeyLY = stateNew.AKeyLY;
		stateNow.AKeyRX = stateNew.AKeyRX;
		stateNow.AKeyRY = stateNew.AKeyRY;

	}

}

InputManager::JOYPAD_IN_STATE InputManager::GetJPadInputState(JOYPAD_NO no)
{

	JOYPAD_IN_STATE ret = JOYPAD_IN_STATE();

	auto type = GetJPadType(no);
	
	switch (type)
	{
	case InputManager::JOYPAD_TYPE::OTHER:
		break;
	case InputManager::JOYPAD_TYPE::XBOX_360:
	{
	}
		break;
	case InputManager::JOYPAD_TYPE::XBOX_ONE:
	{

		auto d = GetJPadDInputState(no);
		auto x = GetJPadXInputState(no);

		int idx;

		//   Y
		// X   B
		//   A

		idx = static_cast<int>(JOYPAD_BTN::START);
		ret.ButtonsNew[idx] = d.Buttons[7];// START

		idx = static_cast<int>(JOYPAD_BTN::BACK);
		ret.ButtonsNew[idx] = d.Buttons[6];// BACK

		idx = static_cast<int>(JOYPAD_BTN::RB);
		ret.ButtonsNew[idx] = d.Buttons[5];// RB

		idx = static_cast<int>(JOYPAD_BTN::LB);
		ret.ButtonsNew[idx] = d.Buttons[4];// LB

		idx = static_cast<int>(JOYPAD_BTN::TOP);
		ret.ButtonsNew[idx] = d.Buttons[3];// Y

		idx = static_cast<int>(JOYPAD_BTN::LEFT);
		ret.ButtonsNew[idx] = d.Buttons[2];// X

		idx = static_cast<int>(JOYPAD_BTN::RIGHT);
		ret.ButtonsNew[idx] = d.Buttons[1];// B

		idx = static_cast<int>(JOYPAD_BTN::DOWN);
		ret.ButtonsNew[idx] = d.Buttons[0];// A

		idx = static_cast<int>(JOYPAD_BTN::R_TRIGGER);
		ret.ButtonsNew[idx] = x.RightTrigger;// R_TRIGGER

		idx = static_cast<int>(JOYPAD_BTN::L_TRIGGER);
		ret.ButtonsNew[idx] = x.LeftTrigger; // L_TRIGGER

		// 左スティック
		ret.AKeyLX = d.X;
		ret.AKeyLY = d.Y;
		
		// 右スティック
		ret.AKeyRX = d.Rx;
		ret.AKeyRY = d.Ry;

	}
		break;
	case InputManager::JOYPAD_TYPE::DUAL_SHOCK_4:
		break;
	case InputManager::JOYPAD_TYPE::DUAL_SENSE:
	{
		
		auto d = GetJPadDInputState(no);
		int idx;

		//   △
		// □  〇
		//   ×

		idx = static_cast<int>(JOYPAD_BTN::TOP);
		ret.ButtonsNew[idx] = d.Buttons[3];// △

		idx = static_cast<int>(JOYPAD_BTN::LEFT);
		ret.ButtonsNew[idx] = d.Buttons[0];// □

		idx = static_cast<int>(JOYPAD_BTN::RIGHT);
		ret.ButtonsNew[idx] = d.Buttons[2];// 〇

		idx = static_cast<int>(JOYPAD_BTN::DOWN);
		ret.ButtonsNew[idx] = d.Buttons[1];// ×

		// 左スティック
		ret.AKeyLX = d.X;
		ret.AKeyLY = d.Y;
		
		// 右スティック
		ret.AKeyRX = d.Z;
		ret.AKeyRY = d.Rz;

	}
		break;
	case InputManager::JOYPAD_TYPE::SWITCH_JOY_CON_L:
		break;
	case InputManager::JOYPAD_TYPE::SWITCH_JOY_CON_R:
		break;
	case InputManager::JOYPAD_TYPE::SWITCH_PRO_CTRL:
		break;
	case InputManager::JOYPAD_TYPE::MAX:
		break;
	}

	return ret;

}

bool InputManager::IsPadBtnNew(JOYPAD_NO no, JOYPAD_BTN btn) const
{
	return padInfos_[static_cast<int>(no)].IsNew[static_cast<int>(btn)];
}

bool InputManager::IsPadBtnTrgDown(JOYPAD_NO no, JOYPAD_BTN btn) const
{
	return padInfos_[static_cast<int>(no)].IsTrgDown[static_cast<int>(btn)];
}

bool InputManager::IsPadBtnTrgUp(JOYPAD_NO no, JOYPAD_BTN btn) const
{
	return padInfos_[static_cast<int>(no)].IsTrgUp[static_cast<int>(btn)];
}

VECTOR InputManager::GetDirectionXZAKey(int aKeyX, int aKeyY)
{
	VECTOR ret = { 0.0f, 0.0f, 0.0f };

	// スティックの個々の入力値は、
	// -1000.0f ～ 1000.0f の範囲で返ってくるが、
	// X:1000.0f、Y:1000.0fになることは無い(1000と500くらいが最大)
	// スティックの入力値を -1.0 ～ 1.0 に正規化
	float dirX = static_cast<float>(aKeyX) / AKEY_VAL_MAX;
	float dirZ = static_cast<float>(aKeyY) / AKEY_VAL_MAX;

	// ピタゴラスの定理でニュートラル状態からの長さベクトルにする
	// ( 円形のデッドゾーンになる )
	// 平方根により、おおよその最大値が1.0となる
	float len = sqrtf(dirX * dirX + dirZ * dirZ);
	if (len < THRESHOLD)
	{
		// (0.0f, 0.0f, 0.0f)
		return ret;
	}

	// デッドゾーン境界からに再スケーリング(可変デッドゾーン)
	// ( しきい値 0.35 の場合は、 0.0 ～ 0.65 / 0.65 になる )
	float scale = (len - THRESHOLD) / (1.0f - THRESHOLD);
	dirX = (dirX / len) * scale;
	dirZ = (dirZ / len) * scale;

	// Zは前に倒すとマイナス値が返ってくるので反転
	ret = VNorm({ dirX, 0.0f, -dirZ });
	return ret;
}

	
