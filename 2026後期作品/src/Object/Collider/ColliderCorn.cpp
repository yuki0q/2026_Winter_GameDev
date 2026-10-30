#include <DxLib.h>
#include "../Common/Transform.h"
#include "ColliderModel.h"
#include "ColliderCorn.h"

ColliderCorn::ColliderCorn(
	TAG tag, const Transform* follow,
	const VECTOR& localPosTop, const VECTOR& localPosDown, float radiusTop, float radiusDown)
	:
	ColliderBase(SHAPE::CORN, tag, follow),
	localPosTop_(localPosTop),
	localPosDown_(localPosDown),
	radiusTop_(radiusTop),
	radiusDown_(radiusDown)
{
}

ColliderCorn::~ColliderCorn(void)
{
}

const VECTOR& ColliderCorn::GetLocalPosTop(void) const
{
	return localPosTop_;
}

const VECTOR& ColliderCorn::GetLocalPosDown(void) const
{
	return localPosDown_;
}

void ColliderCorn::SetLocalPosTop(const VECTOR& pos)
{
	localPosTop_ = pos;
}

void ColliderCorn::SetLocalPosDown(const VECTOR& pos)
{
	localPosDown_ = pos;
}

VECTOR ColliderCorn::GetPosTop(void) const
{
	return GetRotPos(localPosTop_);
}

VECTOR ColliderCorn::GetPosDown(void) const
{
	return GetRotPos(localPosDown_);
}

float ColliderCorn::GetRadiusTop(void) const
{
	return radiusTop_;
}

void ColliderCorn::SetRadiusTop(float radiusTop)
{
	radiusTop_ = radiusTop;
}

float ColliderCorn::GetRadiusDown(void) const
{
	return radiusDown_;
}

void ColliderCorn::SetRadiusDown(float radiusDown)
{
	radiusDown_ = radiusDown;
}

float ColliderCorn::GetHeight(void) const
{
	return localPosTop_.y;
}

VECTOR ColliderCorn::GetCenter(void) const
{
	VECTOR top = GetPosTop();
	VECTOR down = GetPosDown();
	VECTOR diff = VSub(top, down);
	return VAdd(down, VScale(diff, 0.5f));
}

VECTOR ColliderCorn::GetPosPushBackAlongNormal(const MV1_COLL_RESULT_POLY& hitColPoly, int maxTryCnt, float pushDistance) const
{
	// コピー生成
	Transform tmpTransform = *follow_;
	ColliderCorn tmpCorn = *this;
	tmpCorn.SetFollow(&tmpTransform);

	// 衝突補正処理
	int tryCnt = 0;

	while (tryCnt < maxTryCnt)
	{
		// コーンと三角形の当たり判定
		if (!HitCheck_Capsule_Triangle(
			tmpCorn.GetPosTop(), tmpCorn.GetPosDown(),
			tmpCorn.GetRadius(),
			hitColPoly.Position[0], hitColPoly.Position[1],
			hitColPoly.Position[2]))
		{
			break;
		}

		// 衝突していたら法線方向に押し戻し
		tmpTransform.pos =
			VAdd(tmpTransform.pos, VScale(hitColPoly.Normal, pushDistance));

		tryCnt++;
	}

	return tmpTransform.pos;
}

void ColliderCorn::PushBackAlongNormal(const ColliderModel* colliderModel,
	Transform& transform, int maxTryCnt, float pushDistance, bool isExclude, bool isTarget) const
{
	// モデルとカプセルの衝突判定
	auto hits = MV1CollCheck_Capsule(
		colliderModel->GetFollow()->modelId, -1,
		GetPosTop(), GetPosDown(), GetRadius());

	// 衝突した複数のポリゴンと衝突回避するまで、位置を移動させる
	for (int i = 0; i < hits.HitNum; i++)
	{
		auto hitPoly = hits.Dim[i];

		// 除外フレームは無視する
		if (isExclude && colliderModel->IsExcludeFrame(hitPoly.FrameIndex))
		{
			continue;
		}

		// 対象フレーム以外は無視する
		if (isTarget && !colliderModel->IsTargetFrame(hitPoly.FrameIndex))
		{
			continue;
		}

		// 指定された回数と距離で三角形の法線方向に押し戻す
		transform.pos =
			GetPosPushBackAlongNormal(hitPoly, maxTryCnt, pushDistance);
	}
	// 検出した地面ポリゴン情報の後始末
	MV1CollResultPolyDimTerminate(hits);

}

bool ColliderCorn::IsHit(const ColliderModel* colliderModel, bool isExclude, bool isTarget)const
{
	bool ret = false;

	// モデルとコーンの衝突判定
	auto hits = MV1CollCheck_Capsule(
		colliderModel->GetFollow()->modelId, -1,
		GetPosTop(), GetPosDown(), GetRadius());

	// 衝突した複数のポリゴンと衝突回避するまで、位置を移動させる
	for (int i = 0; i < hits.HitNum; i++)
	{
		auto hitPoly = hits.Dim[i];

		// 除外フレームは無視する
		if (isExclude && colliderModel->IsExcludeFrame(hitPoly.FrameIndex))
		{
			continue;
		}

		// 対象フレーム以外は無視する
		if (isTarget && !colliderModel->IsTargetFrame(hitPoly.FrameIndex))
		{
			continue;
		}

		// 衝突している
		ret = true;
		break;
	}
	// 検出した地面ポリゴン情報の後始末
	MV1CollResultPolyDimTerminate(hits);

	return ret;
}

void ColliderCorn::DrawDebug(int color)
{
	// 上の球体
	VECTOR pos1 = GetPosTop();
	DrawSphere3D(pos1, radius_, 5, color, color, false);
	// 下の球体
	VECTOR pos2 = GetPosDown();
	DrawSphere3D(pos2, radius_, 5, color, color, false);
	VECTOR dir;
	VECTOR s;
	VECTOR e;
	// 球体を繋ぐ線(X+)
	dir = follow_->GetRight();
	s = VAdd(pos1, VScale(dir, radius_));
	e = VAdd(pos2, VScale(dir, radius_));
	DrawLine3D(s, e, color);
	// 球体を繋ぐ線(X-)
	dir = follow_->GetLeft();
	s = VAdd(pos1, VScale(dir, radius_));
	e = VAdd(pos2, VScale(dir, radius_));
	DrawLine3D(s, e, color);
	// 球体を繋ぐ線(Z+)
	dir = follow_->GetForward();
	s = VAdd(pos1, VScale(dir, radius_));
	e = VAdd(pos2, VScale(dir, radius_));
	DrawLine3D(s, e, color);
	// 球体を繋ぐ線(Z-)
	dir = follow_->GetBack();
	s = VAdd(pos1, VScale(dir, radius_));
	e = VAdd(pos2, VScale(dir, radius_));
	DrawLine3D(s, e, color);
	// カプセルの中心
	DrawSphere3D(GetCenter(), 5.0f, 10, color, color, true);
}