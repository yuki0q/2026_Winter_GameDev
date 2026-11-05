#include <DxLib.h>
#include "../Utility/AsoUtility.h"
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "../Manager/Camera.h"
#include "../Manager/ResourceManager.h"
#include "../Manager/Resource.h"
#include "../Manager/SoundManager.h"
#include "TitleScene.h"

namespace
{
	const VECTOR TITLE_POS = { -30.0f,0.0f,0.0f };
	const VECTOR TITLE_LOGO_POS = { 0.0f,130.0f,0.0f };

	const float IMG_CX = 0.5f;
	const float IMG_CY = 0.5f;
	const float TITLE_SIZE = 1200.0f;
	const float TITLE_LOGO_SIZE = 450.0f;
}

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
	imgTitleLogo_(),
	SceneBase()
{
}

TitleScene::~TitleScene(void)
{
}

void TitleScene::Init(void)
{
	// 画像読み込み
	imgTitle_ = resMng_.Load(ResourceManager::SRC::IMG_TITLE).handleId_;
	imgTitleLogo_ = resMng_.Load(ResourceManager::SRC::IMG_TITLE_LOGO).handleId_;

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
	bool IsPause = ins.IsActionTrgDown(InputManager::ACTION::PAUSE);
	bool IsScene = ins.IsActionTrgDown(InputManager::ACTION::PARRY);

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
	if (IsPause)
	{
		Application::GetInstance().Shutdown();
	}
	if (IsScene)
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
	DrawBillboard3D(TITLE_POS, IMG_CX, IMG_CY, TITLE_SIZE, 0.0f, imgTitle_, false);
	DrawBillboard3D(TITLE_LOGO_POS, IMG_CX, IMG_CY, TITLE_LOGO_SIZE, 0.0f, imgTitleLogo_, true);

	DrawFormatString(10, 10, 0xffffff, "タイトルシーン");

	DrawFormatString(10, 560, 0xffffff, "ESCキーで終了");
}

void TitleScene::Release(void)
{
	DeleteGraph(imgTitleLogo_);
	DeleteGraph(imgTitle_);
}
