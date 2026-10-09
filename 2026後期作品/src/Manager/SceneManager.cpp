#include <chrono>
#include <DxLib.h>
#include <EffekseerForDXLib.h>
#include "../Common/Fader.h"
#include "../Scene/TitleScene.h"
#include "../Scene/GameScene.h"
#include "../Scene/PauseScene.h"
#include "Camera.h"
//#include "ResourceManager.h"
#include "SceneManager.h"

SceneManager* SceneManager::instance_ = nullptr;

void SceneManager::CreateInstance()
{
	if (instance_ == nullptr)
	{
		instance_ = new SceneManager();
	}
	instance_->Init();
}

SceneManager& SceneManager::GetInstance(void)
{
	return *instance_;
}

void SceneManager::Init(void)
{

	sceneId_ = SCENE_ID::TITLE;
	waitSceneId_ = SCENE_ID::NONE;

	// フェード機能の初期化
	fader_ = new Fader();
	fader_->Init();

	// カメラ
	camera_ = new Camera();
	camera_->Init();

	// 画面遷移中判定
	isSceneChanging_ = false;

	// デルタタイム
	preTime_ = std::chrono::system_clock::now();

	// 3D用の設定
	Init3D();

	// 初期シーンの設定
	DoChangeScene(SCENE_ID::TITLE);
	//DoChangeScene(SCENE_ID::GAME);
}

void SceneManager::Init3D(void)
{

	// 背景色設定
	SetBackgroundColor(
		BACKGROUND_COLOR_R,
		BACKGROUND_COLOR_G,
		BACKGROUND_COLOR_B);

	// Zバッファを有効にする
	SetUseZBuffer3D(true);

	// Zバッファへの書き込みを有効にする
	SetWriteZBuffer3D(true);

	// バックカリングを有効にする
	SetUseBackCulling(true);

	// ライトの設定
	SetUseLighting(true);

	// ライトの設定
	ChangeLightTypeDir({ 0.3f, 1.7f, 0.8f });

	// フォグ設定
	SetFogEnable(true);
	SetFogColor(5, 5, 5);
	SetFogStartEnd(10000.0f, 20000.0f);

}

void SceneManager::Update(void)
{

	if (scene_ == nullptr)
	{
		return;
	}

	// デルタタイム
	auto nowTime = std::chrono::system_clock::now();
	deltaTime_ = static_cast<float>(
		std::chrono::duration_cast<std::chrono::nanoseconds>(nowTime - preTime_).count() / 1000000000.0);
	preTime_ = nowTime;

	// フェード機能の更新
	fader_->Update();
	if (isSceneChanging_)
	{
		// フェード状態の切替処理
		Fade();
	}
	else
	{
		// ポーズ中なら、ポーズ画面だけを更新（背後のゲームは止まる）
		if (sceneId_ == SCENE_ID::PAUSE && pauseScene_ != nullptr)
		{
			pauseScene_->Update();
		}
		else if (scene_ != nullptr)
		{
			scene_->Update(); // 通常時の更新
		}
	}

	// カメラ更新
	camera_->Update();

}

void SceneManager::Draw(void)
{

	// 描画先グラフィック領域の指定
	// (３Ｄ描画で使用するカメラの設定などがリセットされる)
	SetDrawScreen(DX_SCREEN_BACK);

	// 画面を初期化
	ClearDrawScreen();

	// カメラ設定
	camera_->SetBeforeDraw();

	// Effekseerにより再生中のエフェクトを更新する。
	UpdateEffekseer3D();

	if (scene_ != nullptr)
	{
		scene_->Draw();
	}

	// カメラ描画
	camera_->DrawDebug();

	// 3Dの奥行き判定を一時的に無効化する
	SetUseZBuffer3D(FALSE);
	SetWriteZBuffer3D(FALSE);
	
	// Effekseerにより再生中のエフェクトを描画する。
	DrawEffekseer3D();

	// 終わったら3D用に設定を戻す
	SetUseZBuffer3D(TRUE);
	SetWriteZBuffer3D(TRUE);

	// ポーズ中なら、ゲーム画面の上に重ねてポーズ画面を描画する
	if (sceneId_ == SCENE_ID::PAUSE && pauseScene_ != nullptr)
	{
		pauseScene_->Draw();
	}

	// 暗転・明転
	fader_->Draw();

}

void SceneManager::Destroy(void)
{

	// シーンの解放
	if (scene_ != nullptr)
	{
		delete scene_;
	}

	if (pauseScene_ != nullptr)
	{
		delete pauseScene_;
	}

	// フェード機能の解放
	delete fader_;

	camera_->Release();
	delete camera_;


	// インスタンスのメモリ解放
	delete instance_;

}

void SceneManager::ChangeScene(SCENE_ID nextId)
{

	// フェード処理が終わってからシーンを変える場合もあるため、
	// 遷移先シーンをメンバ変数に保持
	waitSceneId_ = nextId;

	if (waitSceneId_ == SCENE_ID::PAUSE ||
		(sceneId_ == SCENE_ID::PAUSE && waitSceneId_ == SCENE_ID::GAME)) 
	{
		isSceneChanging_ = false;
		DoChangeScene(waitSceneId_);
	}
	else 
	{
		// フェードアウト(暗転)を開始する
		fader_->SetFade(Fader::STATE::FADE_OUT);
		isSceneChanging_ = true;
	}

}

SceneManager::SCENE_ID SceneManager::GetSceneID(void)
{
	return sceneId_;
}

float SceneManager::GetDeltaTime(void) const
{
	//return 1.0f / 60.0f;
	return deltaTime_;
}

Camera* SceneManager::GetCamera(void) const
{
	return camera_;
}

SceneManager::SceneManager(void)
{

	sceneId_ = SCENE_ID::NONE;
	waitSceneId_ = SCENE_ID::NONE;

	scene_ = nullptr;
	pauseScene_ = nullptr;
	fader_ = nullptr;

	isSceneChanging_ = false;

	// デルタタイム
	deltaTime_ = 1.0f / 60.0f;

	camera_ = nullptr;

}

void SceneManager::ResetDeltaTime(void)
{
	deltaTime_ = 0.016f;
	preTime_ = std::chrono::system_clock::now();
}

void SceneManager::DoChangeScene(SCENE_ID sceneId)
{

	// ポーズ画面を開くとき
	if (sceneId == SCENE_ID::PAUSE)
	{
		sceneId_ = sceneId; // 現在のIDをPAUSEにする
		pauseScene_ = new PauseScene();
		pauseScene_->Init();
		ResetDeltaTime();
		waitSceneId_ = SCENE_ID::NONE;
		return;
	}

	// ポーズ画面からゲーム画面に戻るとき
	if (sceneId_ == SCENE_ID::PAUSE && sceneId == SCENE_ID::GAME)
	{
		sceneId_ = sceneId; // 現在のIDをGAMEに戻す
		if (pauseScene_ != nullptr)
		{
			delete pauseScene_; // ポーズ画面だけを消去
			pauseScene_ = nullptr;
		}
		ResetDeltaTime();
		waitSceneId_ = SCENE_ID::NONE;
		return;
	}

	

	// シーンを変更する
	sceneId_ = sceneId;

	// 現在のシーンを解放
	if (scene_ != nullptr)
	{
		delete scene_; // 元の通常シーンを解放
		scene_ = nullptr;
	}

	switch (sceneId_)
	{
	case SCENE_ID::TITLE:
		scene_ = new TitleScene();
		break;
	case SCENE_ID::GAME:
		scene_ = new GameScene();
		break;
	case SCENE_ID::PAUSE:
		scene_ = new PauseScene();
		break;
	case SCENE_ID::DEBUG:
		//scene_ = new DebugScene();
		break;
	}

	// 各シーンの初期化
	scene_->Init();

	ResetDeltaTime();

	waitSceneId_ = SCENE_ID::NONE;

}

void SceneManager::Fade(void)
{

	Fader::STATE fState = fader_->GetState();
	switch (fState)
	{
	case Fader::STATE::FADE_IN:
		// 明転中
		if (fader_->IsEnd())
		{
			// 明転が終了したら、フェード処理終了
			fader_->SetFade(Fader::STATE::NONE);
			isSceneChanging_ = false;
		}
		break;
	case Fader::STATE::FADE_OUT:
		// 暗転中
		if (fader_->IsEnd())
		{
			// 完全に暗転してからシーン遷移
			DoChangeScene(waitSceneId_);
			// 暗転から明転へ
			fader_->SetFade(Fader::STATE::FADE_IN);
		}
		break;
	}

}