#pragma once
#include "SceneBase.h"
#include <string>
#include <vector>
#include <map>
#include <functional>

class PauseScene :public SceneBase
{

public:

	// コンストラクタ
	PauseScene(void);
	// デストラクタ
	~PauseScene(void)override;

	void Init(void)override;
	void Update(void)override;
	void Draw(void)override;
	void Release(void)override;

private:

	// デフォルトの選択位置
	static constexpr int DEFAULT_SELECT = 200;

	// ボタンの移動量
	static constexpr int SELECT_MOVE = 120;

	// 選択枠の移動
	int select_;
	int count_;
	bool isStickUpOld;
	bool isStickDownOld;

	// 描画用
	int button_;
	int pushedButton_;
	int returnGame_;
	int returnTitle_;
	int returnSelect_;
	int gameEnd_;
	int selectNow_;
};

