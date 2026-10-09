#include <DxLib.h>
#include "../Application.h"
#include "Resource.h"
#include "ResourceManager.h"

ResourceManager* ResourceManager::instance_ = nullptr;

void ResourceManager::CreateInstance(void)
{
	if (instance_ == nullptr)
	{
		instance_ = new ResourceManager();
	}
	instance_->Init();
}

ResourceManager& ResourceManager::GetInstance(void)
{
	return *instance_;
}

void ResourceManager::Init(void)
{

	// 推奨しませんが、どうしても使いたい方は
	using RES = Resource;
	using RES_T = RES::TYPE;
	static std::string PATH_IMG = Application::PATH_IMAGE;
	static std::string PATH_MDL = Application::PATH_MODEL;
	static std::string PATH_EFF = Application::PATH_EFFECT;
	static std::string PATH_FONT = Application::PATH_FONT;

	Resource* res;

	// タイトル画像
	res = new RES(RES_T::IMG, PATH_IMG + "newTitle.png");
	resourcesMap_.emplace(SRC::TITLE_IMG, res);

	// プレイヤー
	res = new RES(RES_T::MODEL, PATH_MDL + "Charactor/Player/player.mv1");
	resourcesMap_.emplace(SRC::PLAYER, res);

	res = new RES(RES_T::MODEL, PATH_MDL + "Charactor/Weapon/sword.mv1");
	resourcesMap_.emplace(SRC::SWORD, res);

	res = new RES(RES_T::MODEL, PATH_MDL + "Charactor/Weapon/shield.mv1");
	resourcesMap_.emplace(SRC::SHIELD, res);

	res = new RES(RES_T::MODEL, PATH_MDL + "Stage/MainPlanet.mv1");
	resourcesMap_.emplace(SRC::MAIN_STAGE, res);

//	res = new RES(RES_T::FONT, PATH_FONT + "WanpakuRuikaMonoSeikyo-04.TTF");

	//res = new RES(
	//	RES_T::FONT,
	//	PATH_FONT + "WanpakuRuikaMonoSeikyo-04.TTF", // 実際のファイルパス
	//	"わんぱくルイカ等幅清音教漢-０４",           // フォントの登録ファミリー名
	//	24, 3, DX_FONTTYPE_ANTIALIASING);
	//resourcesMap_.emplace(SRC::FONT_1, res);
}

void ResourceManager::Release(void)
{
	for (auto& p : loadedMap_)
	{
		p.second.Release();
	}

	loadedMap_.clear();
}

void ResourceManager::Destroy(void)
{
	Release();
	for (auto& res : resourcesMap_)
	{
		res.second->Release();
		delete res.second;
	}
	resourcesMap_.clear();
	delete instance_;
}

const Resource& ResourceManager::Load(SRC src)
{
	const Resource& res = _Load(src);
	if (res.type_ == Resource::TYPE::NONE)
	{
		return dummy_;
	}
	return res;
}

int ResourceManager::LoadModelDuplicate(SRC src)
{
	Resource& res = _Load(src);
	if (res.type_ == Resource::TYPE::NONE)
	{
		return -1;
	}

	int duId = MV1DuplicateModel(res.handleId_);
	res.duplicateModelIds_.push_back(duId);

	return duId;
}

ResourceManager::ResourceManager(void)
{
}

Resource& ResourceManager::_Load(SRC src)
{

	// ロード済みチェック
	const auto& lPair = loadedMap_.find(src);
	if (lPair != loadedMap_.end())
	{
		return *resourcesMap_.find(src)->second;
	}

	// リソース登録チェック
	const auto& rPair = resourcesMap_.find(src);
	if (rPair == resourcesMap_.end())
	{
		// 登録されていない
		return dummy_;
	}

	// ロード処理
	rPair->second->Load();

	// 念のためコピーコンストラクタ
	loadedMap_.emplace(src, *rPair->second);

	return *rPair->second;

}
