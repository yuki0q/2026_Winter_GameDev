#pragma once
#include "ActorBase.h"

class StageClassRoom : public ActorBase
{
public:

	// 衝突判定種別
	enum class COLLIDER_TYPE
	{
		MODEL = 0,
		MAX,
	};

	StageClassRoom(void);
	virtual ~StageClassRoom(void);

	void Update(void)override;
	void Draw(void)override;
	void Release(void)override;

	// 除外フレーム名称
	//const std::vector<std::string> EXCLUDE_FRAME_NAMES = { "Mush", "Grass", };

protected:

	// リソースロード
	void InitLoad(void)override;

	// 大きさ、回転、座標の初期化
	void InitTransform(void) override;

	// 衝突判定の初期化
	void InitCollider(void) override;

	// アニメーションの初期化
	void InitAnimation(void) override;

	// 初期化後の個別処理
	void InitPost(void)override;

private:

	static constexpr VECTOR STAGE_DEFAULT_POS = { 0.0f, -1250.0f, 0.0f };

	static constexpr VECTOR STAGE_DEFAULT_SCALE = { 15.0f,10.0f,15.0f };

	// 対象フレーム
	const std::vector<std::string> TARGET_FRAME_NAMES =
	{
	"Ground",
	};
};

