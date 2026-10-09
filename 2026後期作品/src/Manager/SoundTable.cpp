#include "SoundTable.h"

// サウンドID から サウンドパスを取得

// システム用
namespace SoundTable_System
{
	static const std::unordered_map<SoundID, std::string> Table =
	{
		{ SoundID::SE_CARSOL, "Data/Music/SE/Carsol.mp3" },
		{ SoundID::SE_BUTTON, "Data/Music/SE/PushBtn.mp3" },
		{ SoundID::SE_CANCEL, "Data/Music/SE/Cancel.mp3" },
		{ SoundID::SE_WINDOW, "Data/Music/SE/Window.mp3" },
	};
}

// タイトル画面用
namespace SoundTable_Title
{
	static const std::unordered_map<SoundID, std::string> Table =
	{
		{ SoundID::BGM_TITLE, "Data/Music/BGM/Title.mp3" },
	};
}

namespace SoundTable_Select
{
	static const std::unordered_map<SoundID, std::string> Table =
	{
		{ SoundID::BGM_SELECT, "Data/Music/BGM/select.mp3" },
	};
}

// ゲーム画面用
namespace SoundTable_Game
{
	static const std::unordered_map<SoundID, std::string> Table =
	{
		{ SoundID::BGM_BATTLE, "Data/Music/BGM/GameScene.mp3" },
		{ SoundID::SE_RUN,	"Data/Music/SE/spining.mp3" },
		{ SoundID::SE_ATTACK, "Data/Music/SE/hit.mp3" },
		{ SoundID::SE_SKILLAT, "Data/Music/SE/spark.mp3" },
		{ SoundID::SE_SKILLDI, "Data/Music/SE/Shield.mp3" },
		{ SoundID::SE_SKILLST, "Data/Music/SE/wind.mp3" },
		{ SoundID::SE_SKILLBL, "Data/Music/SE/blance.mp3" },
		{ SoundID::SE_ATTACKOVER, "Data/Music/SE/K.O..mp3" },
		{ SoundID::SE_SMASH, "Data/Music/SE/smash.mp3" },
	};
}

namespace SoundTable_Result
{
	static const std::unordered_map<SoundID, std::string> Table =
	{
		{ SoundID::BGM_RESULT, "Data/Music/BGM/Result.mp3" },
	};
}
