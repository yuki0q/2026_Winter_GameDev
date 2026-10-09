#pragma once
#include "SceneBase.h"
#include "../Object/Common/Transform.h"

class Stage;
class Player;
class EnemyManager;

class GameScene : public SceneBase
{

public:

	// コンストラクタ
	GameScene(void);

	// デストラクタ
	~GameScene(void) override;

	// 初期化
	void Init(void) override;

	// 更新
	void Update(void) override;

	// 描画
	void Draw(void) override;

	// 解放
	void Release(void) override;

private:

	std::unique_ptr <Stage> stage_;

	std::unique_ptr <Player> player_;


	int shadowMapHandle_;

	int image3;
	int image2;
	int image1;
	int imgWin_;
	int imgBack_;
};
