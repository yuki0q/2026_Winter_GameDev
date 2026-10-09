#pragma once
#include <string>
#include <unordered_map>

// サウンドID
enum class SoundID
{
	BGM_TITLE,
	BGM_SELECT,
	BGM_BATTLE,
	BGM_RESULT,
	SE_CARSOL,
	SE_BUTTON,
	SE_CANCEL,
	SE_WINDOW,
	SE_RUN,
	SE_ATTACK,
	SE_SKILLAT,
	SE_SKILLDI,
	SE_SKILLST,
	SE_SKILLBL,
	SE_ATTACKOVER,
	SE_SMASH,
};

// 読み込むシーン
enum class LoadScene
{
	SYSTEM,		// システム
	TITLE,		// タイトル
	SELECT,		// セレクト
	GAME,		// ゲーム
	RESULT,		// リザルト
};

// サウンド属性
struct SoundData
{
	std::string path;	// サウンドのパス
};

// サウンドテーブル（名称被りや拡張性の為にクラス化)
class SoundTable
{
public:
	// サウンドテーブルのマップ
	// サウンドID から サウンドパスを取得
	static const std::unordered_map<SoundID, SoundData> Table;
};
