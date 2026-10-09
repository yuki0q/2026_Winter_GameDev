#include "PauseScene.h"
#include<DxLib.h>
#include "../Manager/SceneManager.h"
#include "../Manager/InputManager.h"
#include "../Manager/ResourceManager.h"
#include "../Manager/Resource.h"
#include "../Manager/SoundManager.h"
#include "../Application.h"
#include "TitleScene.h"

constexpr int margin_size = 50;	// 端からのサイズ
constexpr int expand_interval = 30;	// 30フレーム

PauseScene::PauseScene(void)
	:
	SceneBase(),
	button_(0),
	pushedButton_(0),
	select_(0),
	count_(0),
	returnGame_(0),
	returnTitle_(0),
	returnSelect_(0),
	gameEnd_(0),
	selectNow_(0),
	isStickUpOld(false),
	isStickDownOld(false)
{
}

PauseScene::~PauseScene(void)
{
}

void PauseScene::Init(void)
{
	isStickUpOld = false;
	isStickDownOld = false;

	select_ = DEFAULT_SELECT;
	count_ = 0;
}

void PauseScene::Update(void)
{
	// シーン遷移
	auto& ins = InputManager::GetInstance();
	if (ins.IsTrgDown(KEY_INPUT_ESCAPE)||
		ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::START))
	{
		SoundManager::GetInstance()->PlaySE(SoundID::SE_CANCEL);
		sceMng_.ChangeScene(SceneManager::SCENE_ID::GAME);
	}

	if (ins.IsTrgDown(KEY_INPUT_RETURN))
	{
		SoundManager::GetInstance()->StopBGM();
		SoundManager::GetInstance()->DeleteSceneSound(LoadScene::GAME);
		sceMng_.ChangeScene(SceneManager::SCENE_ID::TITLE);
	}

	// 接続されているゲームパッド１の情報を取得
	InputManager::JOYPAD_IN_STATE padState =
		ins.GetJPadInputState(InputManager::JOYPAD_NO::PAD1);

	VECTOR inputDir = ins.GetDirectionXZAKey(padState.AKeyLX, padState.AKeyLY);

	// スティックが上/下に一定以上倒されているかの判定フラグ
	// (inputDir.z > 0.5f が「上」、inputDir.z < -0.5f が「下」になります)
	bool isStickUpNow = (inputDir.z > 0.5f);
	bool isStickDownNow = (inputDir.z < -0.5f);

	if (ins.IsTrgDown(KEY_INPUT_S)|| (isStickDownNow && !isStickDownOld)) {
		select_ += SELECT_MOVE;
		SoundManager::GetInstance()->PlaySE(SoundID::SE_CARSOL);
	}
	else if (select_ > 560) {
		select_ = 200;
	}

	if (ins.IsTrgDown(KEY_INPUT_W) || (isStickUpNow && !isStickUpOld)) {
		select_ -= SELECT_MOVE;
		SoundManager::GetInstance()->PlaySE(SoundID::SE_CARSOL);
	}
	else if (select_ < 200) {
		select_ = 560;
	}

	isStickUpOld = isStickUpNow;
	isStickDownOld = isStickDownNow;

	count_ = select_ / SELECT_MOVE;

	if (ins.IsTrgDown(KEY_INPUT_SPACE) ||
		ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DOWN))
	{
		switch (count_) {
		case 1:
			SoundManager::GetInstance()->PlaySE(SoundID::SE_BUTTON);
			sceMng_.ChangeScene(SceneManager::SCENE_ID::GAME);
			break;
		case 2:
			SoundManager::GetInstance()->PlaySE(SoundID::SE_BUTTON);
			sceMng_.ChangeScene(SceneManager::SCENE_ID::TITLE);
			break;
		case 3:
			
		case 4:
			Application::GetInstance().Shutdown();
			break;
		}
	}
}

void PauseScene::Draw(void)
{
	// 画面全体を薄暗くする（アルファ値 120 / 255）
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 120);
	DrawBox(0, 0, 1280, 720, GetColor(0, 0, 0), TRUE); // サイズはApplicationに合わせて調整してください
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	// 「PAUSE」などの文字やUIを描画
	DrawString(600, 60, "ポーズ中", GetColor(255, 255, 255));
	//DrawString(520, 380, "ESCAPEでゲームに戻る", GetColor(255, 255, 255));
	// ボタンの描画
	DrawRotaGraph(Application::SCREEN_SIZE_X / 2, 200, 0.75f, 0.0f, button_, true);
	DrawRotaGraph(Application::SCREEN_SIZE_X / 2, 320, 0.75f, 0.0f, button_, true);
	DrawRotaGraph(Application::SCREEN_SIZE_X / 2, 440, 0.75f, 0.0f, button_, true);
	DrawRotaGraph(Application::SCREEN_SIZE_X / 2, 560, 0.75f, 0.0f, button_, true);

	// 文字の描画
	DrawRotaGraph(Application::SCREEN_SIZE_X / 2, 200, 0.75f, 0.0f, returnGame_, true);
	DrawRotaGraph(Application::SCREEN_SIZE_X / 2, 320, 0.75f, 0.0f, returnTitle_, true);
	DrawRotaGraph(Application::SCREEN_SIZE_X / 2, 440, 0.75f, 0.0f, returnSelect_, true);
	DrawRotaGraph(Application::SCREEN_SIZE_X / 2, 560, 0.75f, 0.0f, gameEnd_, true);

	// 選択中のボタン用枠
	if ((select_ >= 200) && (select_ <= 560)) {
		DrawRotaGraph(Application::SCREEN_SIZE_X / 2, select_, 1.0f, 0.0f, selectNow_, true);
	}
}

void PauseScene::Release(void)
{
	DeleteGraph(button_);
	DeleteGraph(pushedButton_);
	DeleteGraph(returnGame_);
	DeleteGraph(returnTitle_);
	DeleteGraph(returnSelect_);
	DeleteGraph(gameEnd_);
	DeleteGraph(selectNow_);
}