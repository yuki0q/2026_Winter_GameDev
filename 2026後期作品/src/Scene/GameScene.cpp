#include <DxLib.h>
#include <EffekseerForDXLib.h>
#include "../Manager/SceneManager.h"
#include "../Manager/InputManager.h"
#include "../Manager/Camera.h"
#include "../Manager/ResourceManager.h"
#include "../Manager/SoundManager.h"
#include "../Manager/Resource.h"
#include "../Object/Actor/Stage.h"
#include "../Object/Collider/ColliderBase.h"
#include "../Object/Actor/Charactor/Player.h"
#include "../Application.h"
#include "../Effect/EffekseerEffect.h"
#include "GameScene.h"

GameScene::GameScene(void)
	:
	shadowMapHandle_(0),
	SceneBase(),
	image3(0),
	image2(0),
	image1(0),
	imgBack_(0),
	imgWin_(0)
{
}

GameScene::~GameScene(void)
{
	player_ = nullptr;
	stage_ = nullptr;
}

void GameScene::Init(void)
{
	//EffekseerEffect::GetInstance()->CreateInstance();
	//EffekseerEffect::GetInstance()->Init();
	
	player_ = std::make_unique<Player>();
	player_->Init();

	stage_ = std::make_unique<Stage>();
	stage_->Init();

	const ColliderBase* stageCollider =
		stage_->GetOwnCollider(static_cast<int>(Stage::COLLIDER_TYPE::MODEL));

	// ステージモデルのコライダーをプレイヤーに登録
	player_->AddHitCollider(stageCollider);

	// 追従カメラモードに変更
	Camera* camera = sceMng_.GetCamera();
	camera->SetFollow(&player_->GetTransform());
	camera->ChangeMode(Camera::MODE::FOLLOW);
	camera->AddHitCollider(stageCollider);

	// シャドウマップハンドルの作成
	shadowMapHandle_ = MakeShadowMap(2048, 2048);

	// シャドウマップが想定するライトの方向もセット
	SetShadowMapLightDirection(shadowMapHandle_, VGet(0.5f, -0.5f, 0.5f));

	// シャドウマップに描画する範囲を設定
	SetShadowMapDrawArea(shadowMapHandle_, VGet(-5000.0f, -70.0f, -6000.0f), 
		VGet(6000.0f, 1500.0f, 8000.0f));

	/*image3 = resMng_.Load(ResourceManager::SRC::IMAGE_3).handleId_;
	image2 = resMng_.Load(ResourceManager::SRC::IMAGE_2).handleId_;
	image1 = resMng_.Load(ResourceManager::SRC::IMAGE_1).handleId_;
	imgBack_ = resMng_.Load(ResourceManager::SRC::IMAGE_GAMEBACK).handleId_;
	imgWin_ = resMng_.Load(ResourceManager::SRC::IMAGE_WIN).handleId_;*/

	// タイトル画面に必要なサウンドをロード
	SoundManager::GetInstance()->LoadSceneSound(LoadScene::GAME);

	// タイトルBGMを再生（自動でループ再生されます）
	SoundManager::GetInstance()->PlayBGM(SoundID::BGM_BATTLE);
	SoundManager::GetInstance()->SetBgmVolume(160); 
}

void GameScene::Update(void)
{
	// シーン遷移
	auto const& ins = InputManager::GetInstance();
	if (ins.IsTrgDown(KEY_INPUT_ESCAPE) || 
		ins.IsPadBtnTrgDown(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::START))
	{
		SoundManager::GetInstance()->PlaySE(SoundID::SE_WINDOW);
		sceMng_.ChangeScene(SceneManager::SCENE_ID::PAUSE);
	}

	stage_->Update();

	player_->Update();
}

void GameScene::Draw(void)
{
	// デバッグ用
	DrawFormatString(10, 10, 0xffffff, "ゲームシーン");

	// シャドウマップへの描画の準備
	ShadowMap_DrawSetup(shadowMapHandle_);

	stage_->Draw();

	player_->Draw();

	// シャドウマップへの描画を終了
	ShadowMap_DrawEnd();
	
	// 描画に使用するシャドウマップを設定
	SetUseShadowMap(0, shadowMapHandle_);

	stage_->Draw();

	player_->Draw();

	// 描画に使用するシャドウマップの設定を解除
	SetUseShadowMap(0, -1);

	// 3Dの奥行き判定を一時的に無効化する
	SetUseZBuffer3D(FALSE);
	SetWriteZBuffer3D(FALSE);

	// 終わったら3D用に設定を戻す
	SetUseZBuffer3D(TRUE);
	SetWriteZBuffer3D(TRUE);
}

void GameScene::Release(void)
{
	DeleteShadowMap(shadowMapHandle_);

	DeleteGraph(image3);
	DeleteGraph(image2);
	DeleteGraph(image1);
	DeleteGraph(imgBack_);
	DeleteGraph(imgWin_);
	SoundManager::GetInstance()->StopBGM();
	SoundManager::GetInstance()->DeleteSceneSound(LoadScene::GAME);

	//EffekseerEffect::GetInstance()->DeleteInstance();
}