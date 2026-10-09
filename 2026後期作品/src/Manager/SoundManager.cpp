#include "SoundManager.h"

#include <DxLib.h>
#include <algorithm>
#include "SoundTable.cpp"

SoundManager* SoundManager::instance_ = nullptr;

SoundManager::SoundManager(void)
{
	currentBgm_ = static_cast<SoundID>(-1);
	bgmVolume_ = 255;
	seVolume_ = 255;
	masterVolume_ = 255;
}

SoundManager::~SoundManager()
{
	// 念のため削除関数
	DeleteAll();
}

void SoundManager::Init(void)
{
	// 現在再生されているBGM
	currentBgm_ = static_cast<SoundID>(-1);

	bgmVolume_ = 255;		// bgm音量
	seVolume_ = 255;		// se音量
	masterVolume_ = 255;	// master音量
}

void SoundManager::LoadSceneSound(LoadScene scene)
{
	// 空のテーブルを用意
	const std::unordered_map<SoundID, std::string>* table = nullptr;

	// シーンで読み込みたいテーブルを切り替える
	switch (scene)
	{
	case LoadScene::SYSTEM: table = &SoundTable_System::Table; break;
	case LoadScene::TITLE: table = &SoundTable_Title::Table; break;
	case LoadScene::SELECT: table = &SoundTable_Select::Table; break;
	case LoadScene::GAME:  table = &SoundTable_Game::Table; break;
	case LoadScene::RESULT:  table = &SoundTable_Result::Table; break;
	}

	// 存在しないテーブルを選んでいたら処理しない
	if (!table) return;

	// 指定したテーブルのサウンドを全て読み込む
	for (auto& [id, path] : *table)
	{
		// サウンドを読み込む
		handles_[id] = LoadSoundMem(path.c_str());
	}
}

void SoundManager::DeleteSceneSound(LoadScene scene)
{
	// 空のテーブルを用意
	const std::unordered_map<SoundID, std::string>* table = nullptr;

	// シーンで削除したいテーブルを切り替える
	switch (scene)
	{
	case LoadScene::SYSTEM: table = &SoundTable_System::Table; break;
	case LoadScene::TITLE: table = &SoundTable_Title::Table; break;
	case LoadScene::SELECT: table = &SoundTable_Select::Table; break;
	case LoadScene::GAME:  table = &SoundTable_Game::Table; break;
	case LoadScene::RESULT:  table = &SoundTable_Result::Table; break;
	}

	// 存在しないテーブルを選んでいたら処理しない
	if (!table) return;

	// 指定したテーブルのサウンドを削除する
	for (auto& [id, _] : *table)
	{
		// IDからサウンドハンドルを抽出
		auto it = handles_.find(id);

		// サウンドが読み込まれているか？
		if (it != handles_.end())
		{
			// 削除する
			DeleteSoundMem(it->second);
			// 配列からも削除
			handles_.erase(it);
		}
	}
}

void SoundManager::PlayBGM(SoundID id)
{
	// IDからサウンドハンドルを抽出
	auto it = handles_.find(id);

	// サウンドが読み込まれているか？
	if (it == handles_.end())
		// 読み込まれていないので終了
		return;

	// 現在のBGMが同じならスキップ
	if (currentBgm_ == id && CheckSoundMem(it->second))
		return;

	// 別のBGMが再生中なら停止
	if (currentBgm_ != static_cast<SoundID>(-1))
		StopBGM();

	// BGMを更新
	currentBgm_ = id;

	// 実音量を計算
	int volume = static_cast<int>(bgmVolume_ * (masterVolume_ / 255.0f));

	// 音量を変更
	ChangeVolumeSoundMem(volume, it->second);

	// BGMなのでループ再生
	PlaySoundMem(it->second, DX_PLAYTYPE_LOOP, true);
}

void SoundManager::StopBGM()
{
	// 何も再生されていないなら何もしない
	if (currentBgm_ == static_cast<SoundID>(-1))
		return;

	// IDからサウンドハンドルを抽出
	auto it = handles_.find(currentBgm_);

	// サウンドが読み込まれているか？
	if (it != handles_.end())
		// 読み込まれているのでサウンドを止める
		StopSoundMem(it->second);

	// 現在のBGMを再生していない状態に更新
	currentBgm_ = static_cast<SoundID>(-1);
}

void SoundManager::PlaySE(SoundID id)
{
	// IDからサウンドハンドルを抽出
	auto it = handles_.find(id);

	// サウンドが読み込まれているか？
	if (it == handles_.end())
		// 読み込まれていないので終了
		return;

	// 実音量を計算
	int volume = static_cast<int>(seVolume_ * (masterVolume_ / 255.0f));

	// 音量を変更
	ChangeVolumeSoundMem(volume, it->second);

	// SEは複数同時再生を許可
	PlaySoundMem(it->second, DX_PLAYTYPE_BACK, true);
}

void SoundManager::DeleteAll(void)
{
	// サウンドが1つも読み込まれてないなら処理しない
	if (handles_.empty())
		return;

	// 全てのサウンドを検索
	for (auto& [id, handle] : handles_)
	{
		// エラーサウンドかをチェック
		if (CheckSoundMem(handle) != -1)
		{
			// 削除する
			DeleteSoundMem(handle);
		}
	}

	// 配列をクリア
	handles_.clear();
}

void SoundManager::SetBgmVolume(int volume)
{
	// 0 ～ 255 までに収める
	bgmVolume_ = std::clamp(volume, 0, 255);

	// 別のBGMが再生中
	if (currentBgm_ != static_cast<SoundID>(-1))
	{
		// IDからサウンドハンドルを抽出
		auto it = handles_.find(currentBgm_);

		// サウンドが読み込まれているか？
		if (it != handles_.end())
		{
			// 実音量を計算
			int volume = static_cast<int>(bgmVolume_ * (masterVolume_ / 255.0f));

			// 音量を変更
			ChangeVolumeSoundMem(volume, it->second);
		}
	}
}

void SoundManager::SetSeVolume(int volume)
{
	// 0 ～ 255 までに収める
	seVolume_ = std::clamp(volume, 0, 255);
}

void SoundManager::SetMasterVolume(int volume)
{
	// 0 ～ 255 までに収める
	masterVolume_ = std::clamp(volume, 0, 255);

	// 再生中のBGMに即時反映
	if (currentBgm_ != static_cast<SoundID>(-1))
	{
		// IDからサウンドハンドルを抽出
		auto it = handles_.find(currentBgm_);

		// サウンドが読み込まれているか？
		if (it != handles_.end())
		{
			// 実音量を計算
			int volume = static_cast<int>(bgmVolume_ * (masterVolume_ / 255.0f));

			// 音量を変更
			ChangeVolumeSoundMem(volume, it->second);
		}
	}
}
