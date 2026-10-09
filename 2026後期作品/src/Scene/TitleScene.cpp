#include <DxLib.h>
#include "../Utility/AsoUtility.h"
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "../Manager/Camera.h"
#include "../Manager/ResourceManager.h"
#include "../Manager/Resource.h"
#include "../Manager/SoundManager.h"
//#include "../Object/Actor/SkyDome.h"
#include "TitleScene.h"

TitleScene::TitleScene(void)
	:
	imgTitle_(-1),
	imgPushSpace_(-1),
	animController_(nullptr),
	select_(0),
	count_(0),
	introWindow_(false),
	window_(false),
	isStickUpOld(false),
	isStickDownOld(false),
	SceneBase()
{
}

TitleScene::~TitleScene(void)
{
}

void TitleScene::Init(void)
{
	// 画像読み込み
	//imgPushSpace_ = resMng_.Load(ResourceManager::SRC::TITLE_PUSH_SPACE).handleId_;

	// 定点カメラ
	sceMng_.GetCamera()->ChangeMode(Camera::MODE::FIXED_POINT);


	// タイトル画面に必要なサウンドをロード
	SoundManager::GetInstance()->LoadSceneSound(LoadScene::TITLE);

	// タイトルBGMを再生（自動でループ再生されます）
	SoundManager::GetInstance()->PlayBGM(SoundID::BGM_TITLE);
	SoundManager::GetInstance()->SetBgmVolume(160);

}

void TitleScene::Update(void)
{
	// シーン遷移
	auto& ins = InputManager::GetInstance();

	// 接続されているゲームパッド１の情報を取得
	InputManager::JOYPAD_IN_STATE padState =
		ins.GetJPadInputState(InputManager::JOYPAD_NO::PAD1);

	VECTOR inputDir = ins.GetDirectionXZAKey(padState.AKeyLX, padState.AKeyLY);

	// スティックが上/下に一定以上倒されているかの判定フラグ
	// (inputDir.z > 0.5f が「上」、inputDir.z < -0.5f が「下」になります)
	bool isStickUpNow = (inputDir.z > 0.5f);
	bool isStickDownNow = (inputDir.z < -0.5f);

	//if (!window_)
	//{

	//	if (ins.IsTrgDown(KEY_INPUT_DOWN) || 
	//		ins.IsTrgDown(KEY_INPUT_S) || (isStickDownNow && !isStickDownOld)) {
	//		select_ += SELECT_MOVE;
	//		SoundManager::GetInstance()->PlaySE(SoundID::SE_CARSOL);
	//	}
	//	else if (select_ > 680) {
	//		select_ = 480;
	//	}

	//	if (ins.IsTrgDown(KEY_INPUT_UP) || 
	//		ins.IsTrgDown(KEY_INPUT_W) || (isStickUpNow && !isStickUpOld)) {
	//		select_ -= SELECT_MOVE;
	//		SoundManager::GetInstance()->PlaySE(SoundID::SE_CARSOL);
	//	}
	//	else if (select_ < 480) {
	//		select_ = 680;
	//	}

	//	isStickUpOld = isStickUpNow;
	//	isStickDownOld = isStickDownNow;

	//	count_ = (select_ - DEFAULT_SELECT) / SELECT_MOVE;

	//	if ((ins.IsTrgDown(KEY_INPUT_SPACE) ||
	//		ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DOWN))
	//		&& !introWindow_)
	//	{
	//		SoundManager::GetInstance()->PlaySE(SoundID::SE_BUTTON);
	//		switch (count_) {
	//		case 0:
	//			window_ = true;
	//			//
	//			break;
	//		case 1:
	//			//sceMng_.ChangeScene(SceneManager::SCENE_ID::TITLE);
	//			introWindow_ = true;
	//			break;
	//		case 2:
	//			Application::GetInstance().Shutdown();
	//			break;
	//		}
	//	}
	//	else if ((ins.IsTrgDown(KEY_INPUT_SPACE) || ins.IsTrgDown(KEY_INPUT_ESCAPE) ||
	//		ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DOWN))
	//		&& introWindow_)
	//	{
	//		SoundManager::GetInstance()->PlaySE(SoundID::SE_CANCEL);
	//		introWindow_ = false;
	//	}
	//}else
	//{

	//	isStickUpOld = isStickUpNow;
	//	isStickDownOld = isStickDownNow;

	//	// キャンセルボタン（例: BACKSPACEやESCキーなど）でウィンドウを閉じる
	//	if (ins.IsTrgDown(KEY_INPUT_ESCAPE) ||
	//		ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::START))
	//	{
	//		window_ = false;
	//		SoundManager::GetInstance()->PlaySE(SoundID::SE_CANCEL);
	//	}

		// ウィンドウが開いている状態で決定ボタンが押されたら、ここで初めてシーンを遷移させる
	if ((ins.IsTrgDown(KEY_INPUT_ESCAPE)))
	{
		Application::GetInstance().Shutdown();
	}
	if ((ins.IsTrgDown(KEY_INPUT_SPACE) ||
			ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DOWN)))
		{

			// ゲームシーンへ遷移
			SoundManager::GetInstance()->StopBGM();
			SoundManager::GetInstance()->DeleteSceneSound(LoadScene::TITLE);
			sceMng_.ChangeScene(SceneManager::SCENE_ID::GAME);

		}
	//}
}

void TitleScene::Draw(void)
{
	DrawFormatString(10, 10, 0xffffff, "タイトルシーン");

}

void TitleScene::Release(void)
{
}
