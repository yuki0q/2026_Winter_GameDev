#pragma once
#include <DxLib.h>
#include "ColliderBase.h"
class Transform;
class ColliderModel;

class ColliderCorn : public ColliderBase
{
public:

	// コンストラクタ
	ColliderCorn(
		TAG tag, const Transform* follow,
		const VECTOR& localPosTop, const VECTOR& localPosDown, float radiusTop, float radiusDown);

	// デストラクタ
	~ColliderCorn(void);

	// 親Transformからの相対位置を取得
	const VECTOR& GetLocalPosTop(void) const;
	const VECTOR& GetLocalPosDown(void) const;

	// 親Transformからの相対位置をセット
	void SetLocalPosTop(const VECTOR& pos);
	void SetLocalPosDown(const VECTOR& pos);

	// ワールド座標を取得
	VECTOR GetPosTop(void) const;
	VECTOR GetPosDown(void) const;

	// 半径
	float GetRadiusTop(void) const;
	void SetRadiusTop(float radiusTop);
	float GetRadiusDown(void) const;
	void SetRadiusDown(float radiusDown);

	// 高さ
	float GetHeight(void) const;

	// カプセルの中心座標
	VECTOR GetCenter(void) const;

	// 指定された回数と距離で三角形の法線方向に押し戻した座標を取得
	VECTOR GetPosPushBackAlongNormal(
		const MV1_COLL_RESULT_POLY& hitColPoly,
		int maxTryCnt,
		float pushDistance) const override;

	// 指定された回数と距離で三角形の法線方向に押し戻す
	void PushBackAlongNormal(
		const ColliderModel* colliderModel, Transform& transform,
		int maxTryCnt, float pushDistance,
		bool isExclude = false, bool isTarget = false) const;

	// 衝突しているか否か（対モデル）
	bool IsHit(const ColliderModel* colliderModel,
		bool isExclude = false, bool isTarget = false) const;

protected:

	// デバッグ用描画
	void DrawDebug(int color) override;

private:

	// 親Transformからの相対位置(上側)
	VECTOR localPosTop_;

	// 親Transformからの相対位置(下側)
	VECTOR localPosDown_;

	// 半径
	float radiusTop_;
	float radiusDown_;

};

