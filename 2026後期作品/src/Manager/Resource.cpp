#include <DxLib.h>
#include <EffekseerForDXLib.h>
#include <Windows.h>
#include "Resource.h"

Resource::Resource(void)
	:
	type_(TYPE::NONE),
	path_(""),
	numX_(-1),
	numY_(-1),
	sizeX_(-1),
	sizeY_(-1),
	handleId_(-1),
	handleIds_(nullptr)
{
}

Resource::Resource(TYPE type, const std::string& path)
	:
	type_(type),
	path_(path),
	numX_(-1),
	numY_(-1),
	sizeX_(-1),
	sizeY_(-1),
	handleId_(-1),
	handleIds_(nullptr)
{
}

Resource::Resource(TYPE type, const std::string& path, int numX, int numY, int sizeX, int sizeY)
	:
	type_(type),
	path_(path),
	numX_(numX),
	numY_(numY),
	sizeX_(sizeX),
	sizeY_(sizeY),
	handleId_(-1),
	handleIds_(nullptr)
{
}

Resource::Resource(TYPE type, const std::string& fontFilePath, const std::string& fontName, int size, int thickness, int fontType)
	:
	type_(type),
	path_(fontFilePath),     // 読み込むフォントファイルのパス (例: "Resources/Font/myfont.ttf")
	fontName_(fontName),     // システムに認識させるフォント名 (例: "MyFontName")
	fontSize_(size),
	fontThickness_(thickness),
	fontType_(fontType),
	numX_(-1),
	numY_(-1),
	sizeX_(-1),
	sizeY_(-1),
	handleId_(-1),
	handleIds_(nullptr),
	isFontRegistered_(false) 
{
}

Resource::~Resource(void)
{
}

void Resource::Load(void)
{

	switch (type_)
	{
	case Resource::TYPE::IMG:
		// 画像
		handleId_ = LoadGraph(path_.c_str());
		break;

	case Resource::TYPE::IMGS:
		// 複数画像
		handleIds_ = new int[numX_ * numY_];
		LoadDivGraph(
			path_.c_str(),
			numX_ * numY_,
			numX_, numY_,
			sizeX_, sizeY_,
			&handleIds_[0]);
		break;

	case Resource::TYPE::MODEL:
		// モデル
		handleId_ = MV1LoadModel(path_.c_str());
		break;

	case Resource::TYPE::EFFEKSEER:

		handleId_ = LoadEffekseerEffect(path_.c_str());
		break;

	case Resource::TYPE::FONT:

		// フォントファイルパスが指定されている場合は Windows に登録
		if (!path_.empty())
		{
			// AddFontResourceExA を呼び出し、プライベートフォントとしてプロセスに登録
			int result = AddFontResourceExA(path_.c_str(), FR_PRIVATE, NULL);
			if (result > 0)
			{
				isFontRegistered_ = true;
			}
		}

		// DXライブラリ側でフォントハンドルを作成
		// path_ が空なら fontName_ (システムフォント) をそのまま使用します
		handleId_ = CreateFontToHandle(fontName_.c_str(), fontSize_, fontThickness_, fontType_);
		break;

	}

}

void Resource::Release(void)
{

	switch (type_)
	{
	case Resource::TYPE::IMG:
		DeleteGraph(handleId_);
		break;

	case Resource::TYPE::IMGS:
	{
		int num = numX_ * numY_;
		for (int i = 0; i < num; i++)
		{
			DeleteGraph(handleIds_[i]);
		}
		delete[] handleIds_;
	}
	break;

	case Resource::TYPE::MODEL:
	{
		MV1DeleteModel(handleId_);
		auto ids = duplicateModelIds_;
		for (auto id : ids)
		{
			MV1DeleteModel(id);
		}
	}
	break;

	case Resource::TYPE::EFFEKSEER:

		DeleteEffekseerEffect(handleId_);
		break;

	case Resource::TYPE::FONT:
		// フォントハンドルの削除
		if (handleId_ != -1)
		{
			DeleteFontToHandle(handleId_);
			handleId_ = -1;
		}

		// フォントの登録を解除
		if (isFontRegistered_)
		{
			RemoveFontResourceExA(path_.c_str(), FR_PRIVATE, NULL);
			isFontRegistered_ = false;
		}
		break;
	}

}

void Resource::CopyHandle(int* imgs) const
{

	if (handleIds_ == nullptr)
	{
		return;
	}

	int num = numX_ * numY_;
	for (int i = 0; i < num; i++)
	{
		imgs[i] = handleIds_[i];
	}

}
