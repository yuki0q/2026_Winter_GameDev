#pragma once
#include "SceneBase.h"
#include "../Application.h"
#include "../Object/Common/Transform.h"
#include "../Object/Common/AnimationController.h"

class ClearScene : public SceneBase
{

public:

	enum class ANIM_TYPE
	{
		RUN,
		MAX,
	};

	// コンストラクタ
	ClearScene(void);

	// デストラクタ
	~ClearScene(void) override;

	// 初期化
	void Init(void) override;

	// 更新
	void Update(void) override;

	// 描画
	void Draw(void) override;

	// 解放
	void Release(void) override;

private:

};