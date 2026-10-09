#include <DxLib.h>
// WinMain関数
//--------------------------------
int WINAPI WinMain(
	_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
// ウィンドウサイズ
SetGraphMode(640, 480, 32);
ChangeWindowMode(true);
// DxLibの初期化
SetUseDirect3DVersion(DX_DIRECT3D_11);
if (DxLib_Init() == -1)
{
	return -1;
}
// ゲームループ
while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
{
	// 描画スクリーンの設定
	SetDrawScreen(DX_SCREEN_BACK);
	// 描画スクリーンを初期化
	ClearDrawScreen();
	// 描画スクリーンの切替
	ScreenFlip();
}
// DxLibの後始末
if (DxLib_End() == -1)
{
	return -1;
}
return 0;
}