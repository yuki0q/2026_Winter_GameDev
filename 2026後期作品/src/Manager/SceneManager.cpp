#include <chrono>
#include <cmath>
#include <string>
#include <DxLib.h>
#include <EffekseerForDXLib.h>
#include "../Common/Fader.h"
#include "../Scene/TitleScene.h"
#include "../Scene/GameScene.h"
#include "../Scene/PauseScene.h"
#include "../Scene/GameOverScene.h"
#include "../Scene/ClearScene.h"
#include "Camera.h"
//#include "ResourceManager.h"
#include "SceneManager.h"

SceneManager* SceneManager::instance_ = nullptr;

namespace
{
	// 新シーン生成後に、ロード画面の裏で Update を回すフレーム数
	// （位置・カメラ・武器の取り付けなどを落ち着かせる。多いほど安定するが、その間ゲームが進む）
	const int WARMUP_FRAMES = 10;

	// ロード画面の最低表示時間（秒）。一瞬で消えてチラつくのを防ぐ
	const float LOADING_MIN_TIME = 0.8f;

	// ロード画面のフォントサイズ
	const int LOADING_FONT_SIZE = 32;

	// スピナー（回転する点）の設定
	const int   SPINNER_DOT_NUM = 12;
	const float SPINNER_RADIUS = 36.0f;
	const float SPINNER_DOT_SIZE = 5.0f;
	const float SPINNER_SPEED = 12.0f;	// 1秒あたりに進む点の数
}

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

	// ロード画面
	loadPhase_ = LOAD_PHASE::NONE;
	loadTimer_ = 0.0f;
	loadAnimTimer_ = 0.0f;
	warmupCount_ = 0;
	isWarmingUp_ = false;
	loadingFont_ = CreateFontToHandle(nullptr, LOADING_FONT_SIZE, 3, DX_FONTTYPE_ANTIALIASING);

	//// オフスクリーン(メイン描画先)
	//int sx, sy;
	//GetDrawScreenSize(&sx, &sy);
	//mainScreen_ = MakeScreen(sx, sy, FALSE);

	//// カメラシェイク用シェーダー
	//shakePs_ = LoadPixelShader("Data/Shader/CameraShake.pso");
	//shakeCbuf_ = CreateShaderConstantBuffer(sizeof(float) * 4);

	// 初期シーンの設定
	DoChangeScene(SCENE_ID::TITLE);
	//DoChangeScene(SCENE_ID::GAME);

	mainScreen_ = -1;
	shakePs_ = -1;
	shakeCbuf_ = -1;
	shakePower_ = 0.0f;
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
	if (loadPhase_ != LOAD_PHASE::NONE)
	{
		// ロード画面中（新シーンの生成・準備更新）
		UpdateLoading();
	}
	else if (isSceneChanging_)
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

	// ロード画面中はシーンを描画せず、ロード画面だけを表示する
	if (loadPhase_ != LOAD_PHASE::NONE)
	{
		DrawLoading();
		return;
	}

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

	// ロード画面用フォントの解放
	if (loadingFont_ != -1)
	{
		DeleteFontToHandle(loadingFont_);
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
	// ロード画面中（準備更新を含む）の遷移要求は無視する
	if (isWarmingUp_ || loadPhase_ != LOAD_PHASE::NONE)
	{
		return;
	}

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

void SceneManager::SetShakePower(float power)
{
	shakePower_ = power;
}

SceneManager::SceneManager(void)
	:
	mainScreen_(),
	shakePs_(),
	shakeCbuf_(),
	shakePower_()
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

	loadPhase_ = LOAD_PHASE::NONE;
	loadTimer_ = 0.0f;
	loadAnimTimer_ = 0.0f;
	warmupCount_ = 0;
	isWarmingUp_ = false;
	loadingFont_ = -1;
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
	case SCENE_ID::GAMEOVER:
		scene_ = new GameOverScene();
		break;
	case SCENE_ID::CLEAR:
		scene_ = new ClearScene();
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
			// 完全に暗転したらロード画面へ移る
			// （新シーンの生成は、ロード画面を1フレーム描画した後 UpdateLoading で行う）
			loadPhase_ = LOAD_PHASE::PREPARE;
			loadTimer_ = 0.0f;
			loadAnimTimer_ = 0.0f;
		}
		break;
	}
}


void SceneManager::UpdateLoading(void)
{
	loadAnimTimer_ += deltaTime_;

	switch (loadPhase_)
	{
	case LOAD_PHASE::PREPARE:
		// 前フレームでロード画面を描画済み
		// ここで新シーンを生成・初期化する（時間がかかる処理）
		DoChangeScene(waitSceneId_);

		loadPhase_ = LOAD_PHASE::WARMUP;
		loadTimer_ = 0.0f;
		warmupCount_ = 0;
		break;

	case LOAD_PHASE::WARMUP:

		loadTimer_ += deltaTime_;

		// ロード画面で隠したまま新シーンを数フレーム更新して、状態を落ち着かせる
		if (warmupCount_ < WARMUP_FRAMES)
		{
			isWarmingUp_ = true;
			if (scene_ != nullptr)
			{
				scene_->Update();
			}
			isWarmingUp_ = false;
			warmupCount_++;
		}

		// 準備更新が終わり、最低表示時間も経過したら明転へ
		if (warmupCount_ >= WARMUP_FRAMES && loadTimer_ >= LOADING_MIN_TIME)
		{
			loadPhase_ = LOAD_PHASE::NONE;
			ResetDeltaTime();

			// 暗転状態から明転へ（isSceneChanging_ は明転終了まで true のまま）
			fader_->SetFade(Fader::STATE::FADE_IN);
		}
		break;

	default:
		break;
	}
}

void SceneManager::DrawLoading(void)
{
	int sx, sy;
	GetDrawScreenSize(&sx, &sy);

	// 背景（暗転と同じ黒にして、切り替わりを目立たせない）
	DrawBox(0, 0, sx, sy, GetColor(0, 0, 0), TRUE);

	// 回転するスピナー
	const float cx = static_cast<float>(sx) * 0.5f;
	const float cy = static_cast<float>(sy) * 0.5f - 30.0f;
	const int head = static_cast<int>(loadAnimTimer_ * SPINNER_SPEED) % SPINNER_DOT_NUM;

	for (int i = 0; i < SPINNER_DOT_NUM; i++)
	{
		float angle = DX_TWO_PI_F * static_cast<float>(i) / SPINNER_DOT_NUM - DX_PI_F * 0.5f;

		// 先頭の点ほど明るく、後ろの点ほど暗くする
		int behind = (head - i + SPINNER_DOT_NUM) % SPINNER_DOT_NUM;
		int bright = 255 - behind * (220 / SPINNER_DOT_NUM);

		DrawCircleAA(
			cx + cosf(angle) * SPINNER_RADIUS,
			cy + sinf(angle) * SPINNER_RADIUS,
			SPINNER_DOT_SIZE, 16, GetColor(bright, bright, bright), TRUE);
	}

	// 「NOW LOADING」と、増えていくドット
	// 文字位置がドットの数でずれないよう、最大幅の文字列で左端を決める
	const std::string fullText = "NOW LOADING...";
	const int dotNum = static_cast<int>(loadAnimTimer_ * 3.0f) % 4;
	const std::string text = "NOW LOADING" + std::string(dotNum, '.');

	const int textWidth = GetDrawStringWidthToHandle(
		fullText.c_str(), static_cast<int>(fullText.size()), loadingFont_);
	const int textX = (sx - textWidth) / 2;
	const int textY = static_cast<int>(cy + SPINNER_RADIUS) + 40;

	DrawStringToHandle(textX, textY, text.c_str(), GetColor(255, 255, 255), loadingFont_);
}
