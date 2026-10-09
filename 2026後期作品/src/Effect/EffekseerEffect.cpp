#include "EffekseerEffect.h"
#include "../Application.h"

EffekseerEffect* EffekseerEffect::instance_ = nullptr;

EffekseerEffect::EffekseerEffect(void)
{

}

EffekseerEffect::~EffekseerEffect(void)
{

}

void EffekseerEffect::Init(void)
{
	SetUseDirect3DVersion(DX_DIRECT3D_11);

	if (Effekseer_Init(8000) == -1) {
		DxLib_End();
		return;
	}

	SetChangeScreenModeGraphicsSystemResetFlag(false);

	Effekseer_SetGraphicsDeviceLostCallbackFunctions();

	effFireLight_ = LoadEffekseerEffect(
			(Application::PATH_EFFECT + "FireLight.efkefc").c_str());

	if (effFireLight_ == -1) {
		MessageBoxA(NULL, "エフェクトの読み込みに失敗しました。パスやファイルを確認してください。", "エラー", MB_OK);
	}

	effTrailIdleBlue_ = LoadEffekseerEffect(
		(Application::PATH_EFFECT + "BlueIdle.efkefc").c_str());

	if (effTrailIdleBlue_ == -1) {
		MessageBoxA(NULL, "エフェクトの読み込みに失敗しました。パスやファイルを確認してください。", "エラー", MB_OK);
	}

	effTrailDashBlue_ = LoadEffekseerEffect(
		(Application::PATH_EFFECT + "BlueDash.efkefc").c_str());

	if (effTrailDashBlue_ == -1) {
		MessageBoxA(NULL, "エフェクトの読み込みに失敗しました。パスやファイルを確認してください。", "エラー", MB_OK);
	}

	effTrailIdleGreen_ = LoadEffekseerEffect(
		(Application::PATH_EFFECT + "GreenIdle.efkefc").c_str());

	if (effTrailIdleGreen_ == -1) {
		MessageBoxA(NULL, "エフェクトの読み込みに失敗しました。パスやファイルを確認してください。", "エラー", MB_OK);
	}

	effTrailDashGreen_ = LoadEffekseerEffect(
		(Application::PATH_EFFECT + "GreenDash.efkefc").c_str());

	if (effTrailDashGreen_ == -1) {
		MessageBoxA(NULL, "エフェクトの読み込みに失敗しました。パスやファイルを確認してください。", "エラー", MB_OK);
	}

	effTrailIdleYellow_ = LoadEffekseerEffect(
		(Application::PATH_EFFECT + "YellowIdle.efkefc").c_str());

	if (effTrailIdleYellow_ == -1) {
		MessageBoxA(NULL, "エフェクトの読み込みに失敗しました。パスやファイルを確認してください。", "エラー", MB_OK);
	}

	effTrailDashYellow_ = LoadEffekseerEffect(
		(Application::PATH_EFFECT + "YellowDash.efkefc").c_str());

	if (effTrailDashYellow_ == -1) {
		MessageBoxA(NULL, "エフェクトの読み込みに失敗しました。パスやファイルを確認してください。", "エラー", MB_OK);
	}

	effTrailIdleRed_ = LoadEffekseerEffect(
		(Application::PATH_EFFECT + "RedIdle.efkefc").c_str());

	if (effTrailIdleRed_ == -1) {
		MessageBoxA(NULL, "エフェクトの読み込みに失敗しました。パスやファイルを確認してください。", "エラー", MB_OK);
	}

	effTrailDashRed_ = LoadEffekseerEffect(
		(Application::PATH_EFFECT + "RedDash.efkefc").c_str());

	if (effTrailDashRed_ == -1) {
		MessageBoxA(NULL, "エフェクトの読み込みに失敗しました。パスやファイルを確認してください。", "エラー", MB_OK);
	}

	effShield_ = LoadEffekseerEffect(
		(Application::PATH_EFFECT + "Shield.efkefc").c_str());

	if (effShield_ == -1) {
		MessageBoxA(NULL, "エフェクトの読み込みに失敗しました。パスやファイルを確認してください。", "エラー", MB_OK);
	}

	effStamina_ = LoadEffekseerEffect(
		(Application::PATH_EFFECT + "Stamina.efkefc").c_str());

	if (effStamina_ == -1) {
		MessageBoxA(NULL, "エフェクトの読み込みに失敗しました。パスやファイルを確認してください。", "エラー", MB_OK);
	}

	effAttack_ = LoadEffekseerEffect(
		(Application::PATH_EFFECT + "Attack.efkefc").c_str());

	if (effAttack_ == -1) {
		MessageBoxA(NULL, "エフェクトの読み込みに失敗しました。パスやファイルを確認してください。", "エラー", MB_OK);
	}

	effBalance_ = LoadEffekseerEffect(
		(Application::PATH_EFFECT + "Balance.efkefc").c_str());

	if (effBalance_ == -1) {
		MessageBoxA(NULL, "エフェクトの読み込みに失敗しました。パスやファイルを確認してください。", "エラー", MB_OK);
	}
}

void EffekseerEffect::Update(void)
{
}

void EffekseerEffect::Draw(void)
{
	DrawEffekseer3D();
}

void EffekseerEffect::Release(void)
{
	Effkseer_End();
}

void EffekseerEffect::Delete(void)
{
}

void EffekseerEffect::FireLightEffect(VECTOR& pos)
{
	playFireLight_ = PlayEffekseer3DEffect(effFireLight_);

	SetScalePlayingEffekseer3DEffect(playFireLight_, 70.0f, 70.0f, 70.0f);
	SetPosPlayingEffekseer3DEffect(playFireLight_, pos.x, pos.y, pos.z);
}

void EffekseerEffect::TrailIdleStart(TopBase* top, const int type)
{
	if (!top) return;
	TrailStop(top);

	switch (type) {
	case 0:
		playNowTrail_ = PlayEffekseer3DEffect(effTrailIdleBlue_);
		break;
	case 1:
		playNowTrail_ = PlayEffekseer3DEffect(effTrailIdleGreen_);
		break;
	case 2:
		playNowTrail_ = PlayEffekseer3DEffect(effTrailIdleYellow_);
		break;
	case 3:
		playNowTrail_ = PlayEffekseer3DEffect(effTrailIdleRed_);
		break;
	default:
		playNowTrail_ = PlayEffekseer3DEffect(effTrailIdleBlue_);
		break;
	}

	SetScalePlayingEffekseer3DEffect(playNowTrail_, 50.0f, 50.0f, 50.0f);

	activeTrails_[top] = playNowTrail_;
}

void EffekseerEffect::TrailDashStart(TopBase* top, const int type)
{
	if (!top) return;
	TrailStop(top);

	switch (type) {
	case 0:
		playNowTrail_ = PlayEffekseer3DEffect(effTrailDashBlue_);
		break;
	case 1:
		playNowTrail_ = PlayEffekseer3DEffect(effTrailDashGreen_);
		break;
	case 2:
		playNowTrail_ = PlayEffekseer3DEffect(effTrailDashYellow_);
		break;
	case 3:
		playNowTrail_ = PlayEffekseer3DEffect(effTrailDashRed_);
		break;
	default:
		playNowTrail_ = PlayEffekseer3DEffect(effTrailDashBlue_);
		break;
	}

	SetScalePlayingEffekseer3DEffect(playNowTrail_, 70.0f, 70.0f, 70.0f);

	activeTrails_[top] = playNowTrail_;
}

void EffekseerEffect::TrailUpdate(TopBase* top, VECTOR& pos)
{
	if (!top) return;

	auto it = activeTrails_.find(top);
	if (it != activeTrails_.end()) {
		int handle = it->second; // 保存されていたハンドルを取得

		// エフェクトがまだ再生中なら位置を更新
		if (IsEffekseer3DEffectPlaying(handle) == 0) {
			SetPosPlayingEffekseer3DEffect(handle, pos.x, pos.y, pos.z);
		}
	}
}

void EffekseerEffect::TrailStop(TopBase* top)
{
	if (!top) return;

	// マップからこのコマのエフェクトを探して止める
	auto it = activeTrails_.find(top);
	if (it != activeTrails_.end()) {
		StopEffekseer3DEffect(it->second);
		activeTrails_.erase(it); // 止め終わったらマップから削除
	}
}

void EffekseerEffect::SkillStart(TopBase* top, const int type)
{
	if (!top) return;
	TrailStop(top);

	switch (type) {
	case 0:
		playSkill_ = PlayEffekseer3DEffect(effAttack_);
		break;
	case 1:
		playSkill_ = PlayEffekseer3DEffect(effShield_);
		break;
	case 2:
		playSkill_ = PlayEffekseer3DEffect(effStamina_);
		break;
	case 3:
		playSkill_ = PlayEffekseer3DEffect(effBalance_);
		break;
	default:
		playSkill_ = PlayEffekseer3DEffect(effShield_);
		break;
	}

	SetScalePlayingEffekseer3DEffect(playSkill_, 100.0f, 100.0f, 100.0f);

	activeSkills_[top] = playSkill_;
}

void EffekseerEffect::SkillUpdate(TopBase* top, VECTOR& pos)
{
	if (!top) return;

	auto it = activeSkills_.find(top);
	if (it != activeSkills_.end()) {
		int handle = it->second; // 保存されていたハンドルを取得

		// エフェクトがまだ再生中なら位置を更新
		if (IsEffekseer3DEffectPlaying(handle) == 0) {
			SetPosPlayingEffekseer3DEffect(handle, pos.x, pos.y, pos.z);
		}
	}
}

void EffekseerEffect::SkillStop(TopBase* top)
{
	if (!top) return;

	// マップからこのコマのエフェクトを探して止める
	auto it = activeSkills_.find(top);
	if (it != activeSkills_.end()) {
		StopEffekseer3DEffect(it->second);
		activeSkills_.erase(it); // 止め終わったらマップから削除
	}
}

void EffekseerEffect::ClearAllTrails(void)
{
	// マップに登録されているすべてのエフェクトを停止する
	for (auto& pair : activeTrails_)
	{
		int handle = pair.second;
		if (handle != -1)
		{
			StopEffekseer3DEffect(handle);
		}
	}
	// リストを完全に空にする
	activeTrails_.clear();
}

//void EffekseerEffect::PlayComboEffect6(const VECTOR& pos, float rotY)
//{
//    playFinisyu6Handle = PlayEffekseer3DEffect(finisyu6Id);
//
//    SetPosPlayingEffekseer3DEffect(
//        playFinisyu6Handle,
//        pos.x,
//        pos.y,
//        pos.z
//    );
//
//    SetRotationPlayingEffekseer3DEffect(
//        playFinisyu6Handle,
//        0.0f,
//        rotY,
//        0.0f
//    );
//
//    SetScalePlayingEffekseer3DEffect(
//        playFinisyu6Handle,
//        50.0f, 50.0f, 50.0f
//    );
//
//    SetSpeedPlayingEffekseer3DEffect(
//        playFinisyu6Handle,
//        0.2f
//    );
//}
