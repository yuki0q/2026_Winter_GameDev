#include <DxLib.h>
#include <EffekseerForDXLib.h>
#include "Manager/InputManager.h"
#include "Manager/ResourceManager.h"
#include "Manager/SceneManager.h"
#include "Manager/SoundManager.h"
#include "Effect/EffekseerEffect.h"
#include "Common/FrameLimiter.h"
#include "Application.h"

Application* Application::instance_ = nullptr;

const std::string Application::PATH_IMAGE = "Data/Image/";
const std::string Application::PATH_MODEL = "Data/Model/";
const std::string Application::PATH_EFFECT = "Data/Effect/";
const std::string Application::PATH_CSV = "Data/Csv/";
const std::string Application::PATH_MUSIC = "Data/Music/";
const std::string Application::PATH_SHADER = "Data/Shader/";
const std::string Application::PATH_FONT = "Data/Font/";

void Application::CreateInstance(void)
{
	if (instance_ == nullptr)
	{
		instance_ = new Application();
	}
	instance_->Init();
}

Application& Application::GetInstance(void)
{
	return *instance_;
}

void Application::Init(void)
{

	// アプリケーションの初期設定
	SetWindowText("2416007 押川征矢");

	// ウィンドウサイズ
	SetGraphMode(SCREEN_SIZE_X, SCREEN_SIZE_Y, 32);
	ChangeWindowMode(true);

	// FPS制御初期化
	frameLimiter_ = new FrameLimiter(FRAME_RATE);

	// フレームレートは FrameLimiter で制御するので、垂直同期による待ちは使わない
	// （二重に待つと、リフレッシュレートが高いモニターでは、カクつきや無駄な待ちが出る）
	SetWaitVSyncFlag(FALSE);

	// DxLibの初期化
	SetUseDirect3DVersion(DX_DIRECT3D_11);
	isInitFail_ = false;
	if (DxLib_Init() == -1)
	{
		isInitFail_ = true;
		return;
	}

	// Effekseerの初期化
	InitEffekseer();

	// 乱数のシード値を設定する
	DATEDATA date;

	// 現在時刻を取得する
	GetDateTime(&date);

	// 乱数の初期値を設定する
	// 設定する数値によって、ランダムの出方が変わる
	SRand(date.Year + date.Mon + date.Day + date.Hour + date.Min + date.Sec);

	// 入力制御初期化
	SetUseDirectInputFlag(true);
	InputManager::CreateInstance();

	// リソース管理初期化
	ResourceManager::CreateInstance();

	SoundManager::CreateInstance();
	SoundManager::GetInstance()->Init();
	SoundManager::GetInstance()->LoadSceneSound(LoadScene::SYSTEM);

	// シーン管理初期化
	SceneManager::CreateInstance();

	EffekseerEffect::CreateInstance();
}

void Application::Run(void)
{

	InputManager& inputManager = InputManager::GetInstance();
	SceneManager& sceneManager = SceneManager::GetInstance();

	// ゲームループ
	while (ProcessMessage() == 0 && !isShutdownRequested_)
	{

		inputManager.Update();
		sceneManager.Update();

		sceneManager.Draw();

#ifdef _DEBUG
		// 平均FPS描画
		DrawFormatString(SCREEN_SIZE_X - 140, 10, 0xffffff, "FPS: %.1f", frameLimiter_->GetFps());
#endif // _DEBUG

		ScreenFlip();

		// 理想FPS経過待ち（待っている間は Sleep で寝て、CPUを使わない）
		frameLimiter_->Wait();
	}

}

void Application::Destroy(void)
{
	// FPS制御メモリ解放
	delete frameLimiter_;

	InputManager::GetInstance().Destroy();
	ResourceManager::GetInstance().Destroy();
	SoundManager::DeleteInstance();

	// シーン管理解放
	SceneManager::GetInstance().Destroy();
	EffekseerEffect::GetInstance()->DeleteInstance();

	// Effekseerを終了する。
	Effkseer_End();

	// DxLib終了
	if (DxLib_End() == -1)
	{
		isReleaseFail_ = true;
	}

	// インスタンスのメモリ解放
	delete instance_;

}

bool Application::IsInitFail(void) const
{
	return isInitFail_;
}

bool Application::IsReleaseFail(void) const
{
	return isReleaseFail_;
}

Application::Application(void)
	:
	isInitFail_(false),
	isReleaseFail_(false),
	frameLimiter_(nullptr)
{
}

void Application::InitEffekseer(void)
{
	if (Effekseer_Init(8000) == -1)
	{
		DxLib_End();
	}

	SetChangeScreenModeGraphicsSystemResetFlag(FALSE);

	Effekseer_SetGraphicsDeviceLostCallbackFunctions();
}

void Application::Shutdown()
{
	isShutdownRequested_ = true;
}