#include <cmath>
#include "../../../Manager/SceneManager.h"
#include "../../../Utility/AsoUtility.h"
#include "MagicProjectile.h"

MagicProjectile::MagicProjectile(const VECTOR& pos, const VECTOR& dir, int damage,
	const EnemyMagicParam& param)
	:
	param_(param),
	damage_(damage),
	dir_(dir),
	traveled_(0.0f),
	age_(0.0f),
	isDead_(false)
{

	// 進行方向を正規化（方向が決まらなければ +Z）
	if (VSize(dir_) < 0.001f)
	{
		dir_ = AsoUtility::DIR_F;
	}
	dir_ = AsoUtility::VNormalize(dir_);

	transform_.pos = pos;
	transform_.scl = AsoUtility::VECTOR_ONE;
	transform_.quaRot = Quaternion::Identity();
	transform_.Update();

	// 当たり判定は、球（長さがほぼ0のカプセル）
	// ※完全に長さ0だと判定の計算が不安定になる可能性があるので、わずかに縦へ伸ばしておく
	collider_ = std::make_unique<ColliderCapsule>(
		ColliderBase::TAG::WEAPON, &transform_,
		VGet(0.0f, 1.0f, 0.0f), VGet(0.0f, -1.0f, 0.0f), param_.radius);

}

MagicProjectile::~MagicProjectile(void)
{
}

void MagicProjectile::Update(void)
{

	if (isDead_) return;

	const float dt = SceneManager::GetInstance().GetDeltaTime();
	age_ += dt;

	// 進行方向へ移動（速度は秒速）
	float move = param_.speed * dt;
	transform_.pos = VAdd(transform_.pos, VScale(dir_, move));
	traveled_ += move;

	// コライダが参照する Transform を更新
	transform_.Update();

	// 飛距離に達したら消える
	if (traveled_ >= param_.range)
	{
		isDead_ = true;
	}

}

void MagicProjectile::Draw(void)
{

	if (isDead_) return;

	// 終盤は徐々に薄くして消す
	float rate = traveled_ / param_.range;
	float fade = 1.0f;
	if (rate > FADE_START_RATE)
	{
		fade = (1.0f - rate) / (1.0f - FADE_START_RATE);
	}
	int alpha = static_cast<int>(255.0f * fade);
	if (alpha <= 0) return;

	// 脈打つように、大きさを少し変える
	float pulse = 1.0f + sinf(age_ * PULSE_SPEED) * PULSE_SIZE;

	// 光る表現にするため、ライティングを切って加算合成で描画する
	SetUseLighting(FALSE);
	SetWriteZBuffer3D(FALSE);

	// 外側の光
	SetDrawBlendMode(DX_BLENDMODE_ADD, alpha / 2);
	unsigned int glowColor = GetColor(170, 70, 255);
	DrawSphere3D(transform_.pos, param_.radius * GLOW_RATE * pulse, 16, glowColor, glowColor, TRUE);

	// 中心の核
	SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
	unsigned int coreColor = GetColor(255, 230, 255);
	DrawSphere3D(transform_.pos, param_.radius * CORE_RATE * pulse, 12, coreColor, coreColor, TRUE);

	// 元の描画設定へ戻す
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	SetWriteZBuffer3D(TRUE);
	SetUseLighting(TRUE);

}

void MagicProjectile::DrawDebug(void)
{
	if (isDead_ || !collider_) return;
	collider_->Draw();
}
