#pragma once
#include "UIBase.h"
#include "../Charactor/Player.h"
#include "../Charactor/Enemy/EnemyManager.h"
#include <memory>


class UIGame : public UIBase
{
public:

	UIGame(std::shared_ptr<Player> player, std::shared_ptr<EnemyManager> enemyManager);
	~UIGame(void) override;

	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;
	void Release(void) override;

private:

	std::shared_ptr<Player> player_;
	std::shared_ptr<EnemyManager> enemyManager_;

	// 必要に応じてUI用の画像ハンドルなどを保持

	// int imgHpBar_;
};