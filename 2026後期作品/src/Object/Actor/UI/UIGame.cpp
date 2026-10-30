#include <DxLib.h>
#include "UIGame.h"
#include "../Charactor/Player.h"
#include "../Charactor/Enemy/EnemyManager.h"

UIGame::UIGame(std::shared_ptr<Player> player, std::shared_ptr<EnemyManager> enemyManager)
	: 
	player_(player), 
	enemyManager_(enemyManager)
{
}

UIGame::~UIGame(void)
{
}

void UIGame::Init(void)
{
	// UIで使用する画像やフォントのロードなどがあればここに記述
}

void UIGame::Update(void)
{
	// UI側で毎フレーム監視・アニメーションさせる処理があればここに記述
}

void UIGame::Draw(void)
{
	// プレイヤーの体力を描画
	if (!player_)return;

	int playerHp = player_->GetHP();
	int playerHpMax = player_->GetHPMax();

	// 簡易的なHPゲージの描画例
	DrawBox(30, 60, 30 + playerHp * 2, 80, 0x00ff00, TRUE);
	DrawBox(30, 60, 30 + playerHpMax * 2, 80, 0xffffff, FALSE); // 枠線

	// 敵の体力や数などを描画する場合
	if (!enemyManager_)return;

	int enemyCount = 0;
	for (const auto& enemy : enemyManager_->GetEnemies())
	{
		if (!enemy || enemy->IsDead())continue;
			
		enemyCount++;
			
		VECTOR enemyPos = enemy->GetTransform().pos;

		// 敵の足元から少し上の位置（頭上）を計算（高さはモデルのサイズに合わせて調整してください）
		VECTOR headPos = VAdd(enemyPos, VGet(0.0f, 200.0f, 0.0f));

		// 3Dのワールド座標を 2D のスクリーン座標に変換
		VECTOR screenPos = ConvWorldPosToScreenPos(headPos);

		// screenPos.z が 0.0f未満、または 1.0f以上、あるいは画面外の場合は描画しない（カメラの背後や画面外）
		if (screenPos.z >= 0.0f && screenPos.z < 1.0f)
		{
			int drawX = static_cast<int>(screenPos.x);
			int drawY = static_cast<int>(screenPos.y);

			// 簡易的な敵のHPバー（幅: 60ピクセル、高さ: 6ピクセル）を頭上に表示
			int barWidth = enemy->GetHPMax();
			int barHeight = 6;
			int startX = drawX - (barWidth / 2);
			int startY = drawY;

			float enemyHpRate = enemy->GetHP(); 

			// 背景（黒/赤枠）
			DrawBox(startX, startY, startX + barWidth, startY + barHeight, 0x333333, TRUE);
			// HP残量（緑）
			DrawBox(startX, startY, startX + enemyHpRate, startY + barHeight, 0x00ff00, TRUE);
			// 外枠
			DrawBox(startX, startY, startX + barWidth, startY + barHeight, 0xffffff, FALSE);
		}

	}

	DrawFormatString(30, 100, 0xffffff, "残りの敵数 : %d", enemyCount);
}

void UIGame::Release(void)
{
	// ロードした画像の削除など
}