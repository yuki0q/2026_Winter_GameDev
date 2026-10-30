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

	effAttackLight_ = LoadEffekseerEffect(
			(Application::PATH_EFFECT + "AttackLight.efkefc").c_str());

	if (effAttackLight_ == -1) {
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

void EffekseerEffect::FireLightEffect(const VECTOR& pos, const VECTOR& scl, const VECTOR& rot)
{
	playFireLight_ = PlayEffekseer3DEffect(effFireLight_);

	SetScalePlayingEffekseer3DEffect(playFireLight_, scl.x, scl.y, scl.z);
	SetPosPlayingEffekseer3DEffect(playFireLight_, pos.x, pos.y, pos.z);

	SetRotationPlayingEffekseer3DEffect(
		playFireLight_,	rot.x,	rot.y,	rot.z);
}

void EffekseerEffect::AttackLightEffect(const VECTOR& pos, const VECTOR& scl, const VECTOR& rot)
{
	playAtackLight_ = PlayEffekseer3DEffect(effAttackLight_);

	SetScalePlayingEffekseer3DEffect(playAtackLight_, scl.x, scl.y, scl.z);
	SetPosPlayingEffekseer3DEffect(playAtackLight_, pos.x, pos.y, pos.z);

	SetRotationPlayingEffekseer3DEffect(
		playAtackLight_, rot.x, rot.y, rot.z);
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
