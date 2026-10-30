#include <DxLib.h>
#include "../Utility/AsoUtility.h"
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "../Manager/Camera.h"
#include "../Manager/ResourceManager.h"
#include "../Manager/Resource.h"
#include "../Manager/SoundManager.h"
#include "ClearScene.h"

ClearScene::ClearScene(void)
{
}

ClearScene::~ClearScene(void)
{
}

void ClearScene::Init(void)
{
	// 定点カメラ
	sceMng_.GetCamera()->ChangeMode(Camera::MODE::FIXED_POINT);
}

void ClearScene::Update(void)
{
	// シーン遷移
	auto& ins = InputManager::GetInstance();

	// 接続されているゲームパッド１の情報を取得
	InputManager::JOYPAD_IN_STATE padState =
		ins.GetJPadInputState(InputManager::JOYPAD_NO::PAD1);

	if ((ins.IsTrgDown(KEY_INPUT_ESCAPE)))
	{
		Application::GetInstance().Shutdown();
	}
	if ((ins.IsTrgDown(KEY_INPUT_SPACE) ||
		ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DOWN)))
	{

		// ゲームシーンへ遷移
		SoundManager::GetInstance()->StopBGM();
		SoundManager::GetInstance()->DeleteSceneSound(LoadScene::GAME);
		sceMng_.ChangeScene(SceneManager::SCENE_ID::TITLE);

	}
}

void ClearScene::Draw(void)
{
	DrawFormatString(10, 10, 0xffffff, "ゲームクリアシーン");
}

void ClearScene::Release(void)
{
}
