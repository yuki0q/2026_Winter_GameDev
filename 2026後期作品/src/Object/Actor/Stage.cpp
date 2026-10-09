#include "../../Utility/AsoUtility.h"
#include "../../Manager/ResourceManager.h"
#include "../../Manager/Resource.h"
#include "../../Manager/SceneManager.h"
#include "../Collider/ColliderModel.h"
#include "Stage.h"

Stage::Stage(void)
	:
	ActorBase()
{
}

Stage::~Stage(void)
{
}

void Stage::Update(void)
{
}

void Stage::Draw(void)
{
	ActorBase::Draw();
}

void Stage::Release(void)
{
	ActorBase::Release();
}

void Stage::InitLoad(void)
{
	transform_.SetModel(resMng_.Load(ResourceManager::SRC::MAIN_STAGE).handleId_);
}

void Stage::InitTransform(void)
{
	transform_.scl = STAGE_DEFAULT_SCALE;
	transform_.quaRot = Quaternion::Identity();

	transform_.quaRotLocal = Quaternion::Identity();

	transform_.pos = STAGE_DEFAULT_POS;
	transform_.Update();
}

void Stage::InitCollider(void)
{
	// DxLib側の衝突情報セットアップ
	MV1SetupCollInfo(transform_.modelId);

	// モデルのコライダ
	ColliderModel* colModel =
		new ColliderModel(ColliderBase::TAG::STAGE, &transform_);

	ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::MODEL), colModel);
}

void Stage::InitAnimation(void)
{
}

void Stage::InitPost(void)
{
}
