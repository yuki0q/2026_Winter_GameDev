#include <cmath>
#include "../../../Manager/SceneManager.h"
#include "../../../Utility/AsoUtility.h"
#include "SlashProjectile.h"

SlashProjectile::SlashProjectile(const VECTOR& pos, const VECTOR& dir, const PlayerSlashParam& param)
	:
	param_(param),
	dir_(dir),
	lateral_(),
	traveled_(0.0f),
	isDead_(false)
{

	// 進行方向は水平に正規化（方向が決まらなければ +Z）
	dir_.y = 0.0f;
	if (VSize(dir_) < 0.001f)
	{
		dir_ = AsoUtility::DIR_F;
	}
	dir_ = AsoUtility::VNormalize(dir_);

	// 進行方向と直角な横方向（水平）
	lateral_ = AsoUtility::VNormalize(VCross(VGet(0.0f, 1.0f, 0.0f), dir_));

	transform_.pos = pos;
	transform_.scl = AsoUtility::VECTOR_ONE;
	transform_.quaRot = Quaternion::Identity();
	transform_.Update();

	// 当たり判定は、横方向に伸びたカプセル
	// 回転に依存しないよう、横方向のベクトルを直接ローカル座標として与える
	VECTOR localTop = VScale(lateral_, -param_.width);
	VECTOR localDown = VScale(lateral_, param_.width);

	collider_ = std::make_unique<ColliderCapsule>(
		ColliderBase::TAG::WEAPON, &transform_, localTop, localDown, param_.radius);

}

SlashProjectile::~SlashProjectile(void)
{
}

void SlashProjectile::Update(void)
{

	if (isDead_) return;

	// 進行方向へ移動（速度は秒速）
	float move = param_.speed * SceneManager::GetInstance().GetDeltaTime();
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

void SlashProjectile::Draw(void)
{

	if (isDead_) return;

	// 終盤は徐々に薄くして消す
	float rate = traveled_ / param_.range;
	float fade = 1.0f;
	if (rate > FADE_START_RATE)
	{
		fade = (1.0f - rate) / (1.0f - FADE_START_RATE);
	}
	int alpha = static_cast<int>(200.0f * fade);
	if (alpha <= 0) return;

	// 光る表現にするため、ライティングを切って加算合成で描画する
	SetUseLighting(FALSE);
	SetWriteZBuffer3D(FALSE);
	SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);

	// 上下に少しずらした3枚を重ねて、厚みを出す
	DrawCrescent(-LAYER_GAP, alpha);
	DrawCrescent(0.0f, alpha);
	DrawCrescent(LAYER_GAP, alpha);

	// 元の描画設定へ戻す
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	SetWriteZBuffer3D(TRUE);
	SetUseLighting(TRUE);

}

void SlashProjectile::DrawDebug(void)
{
	if (isDead_ || !collider_) return;
	collider_->Draw();
}

void SlashProjectile::DrawCrescent(float offsetY, int alpha) const
{

	const float width = param_.width;
	const float depth = width * CRESCENT_DEPTH_RATE;
	const unsigned int bodyColor = GetColor(120, 210, 255);
	const unsigned int edgeColor = GetColor(220, 255, 255);

	// 横位置 t（-1 から 1）における、外側（前方）と内側（後方）の曲線の前後位置を求めて、面を並べる
	// 外側: depth * (1 - t^2)   内側: depth * INNER_RATE * (1 - t^2)   （両端 t=±1 で一致 → 三日月）
	auto calcPoint = [&](float t, float depthRate) -> VECTOR
	{
		float forward = depth * depthRate * (1.0f - t * t);
		VECTOR p = transform_.pos;
		p = VAdd(p, VScale(lateral_, width * t));
		p = VAdd(p, VScale(dir_, forward));
		p.y += offsetY;
		return p;
	};

	for (int i = 0; i < CRESCENT_SEGMENTS; i++)
	{
		float t0 = -1.0f + 2.0f * static_cast<float>(i) / CRESCENT_SEGMENTS;
		float t1 = -1.0f + 2.0f * static_cast<float>(i + 1) / CRESCENT_SEGMENTS;

		VECTOR outer0 = calcPoint(t0, 1.0f);
		VECTOR outer1 = calcPoint(t1, 1.0f);
		VECTOR inner0 = calcPoint(t0, CRESCENT_INNER_RATE);
		VECTOR inner1 = calcPoint(t1, CRESCENT_INNER_RATE);

		// バックカリングで消えないよう、表裏の両方を描画
		DrawTriangle3D(outer0, outer1, inner0, bodyColor, TRUE);
		DrawTriangle3D(inner0, outer1, outer0, bodyColor, TRUE);
		DrawTriangle3D(inner0, outer1, inner1, bodyColor, TRUE);
		DrawTriangle3D(inner1, outer1, inner0, bodyColor, TRUE);

		// 外側の縁を明るく
		DrawLine3D(outer0, outer1, edgeColor);
	}

}
