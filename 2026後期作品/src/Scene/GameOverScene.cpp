#include <DxLib.h>
#include "../Utility/AsoUtility.h"
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "../Manager/Camera.h"
#include "../Manager/ResourceManager.h"
#include "../Manager/Resource.h"
#include "../Manager/SoundManager.h"
#include "GameOverScene.h"

GameOverScene::GameOverScene(void)
{
}

GameOverScene::~GameOverScene(void)
{
}

void GameOverScene::Init(void)
{
	// 定点カメラ
	sceMng_.GetCamera()->ChangeMode(Camera::MODE::FIXED_POINT);
}

void GameOverScene::Update(void)
{
	// シーン遷移
	auto& ins = InputManager::GetInstance();
	bool IsPause = ins.IsActionTrgDown(InputManager::ACTION::PAUSE);
	bool IsScene = ins.IsActionTrgDown(InputManager::ACTION::PARRY);
	// 接続されているゲームパッド１の情報を取得
	InputManager::JOYPAD_IN_STATE padState =
		ins.GetJPadInputState(InputManager::JOYPAD_NO::PAD1);

	if (IsPause)
	{
		Application::GetInstance().Shutdown();
	}
	if (IsScene)
	{

		// ゲームシーンへ遷移
		SoundManager::GetInstance()->StopBGM();
		SoundManager::GetInstance()->DeleteSceneSound(LoadScene::GAME);
		sceMng_.ChangeScene(SceneManager::SCENE_ID::TITLE);

	}
}

void GameOverScene::Draw(void)
{
	DrawFormatString(10, 10, 0xffffff, "ゲームオーバーシーン");

	if (GetJoypadNum() == 0)
	{
		DrawFormatString(10, 500, 0xffffff, "SPACEでタイトルへ");
	}
	else
	{
		DrawFormatString(10, 500, 0xffffff, "Bでタイトルへ");
	}
}

void GameOverScene::Release(void)
{
}
