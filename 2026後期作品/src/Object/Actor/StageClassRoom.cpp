#include "../../Utility/AsoUtility.h"
#include "../../Manager/ResourceManager.h"
#include "../../Manager/Resource.h"
#include "../../Manager/SceneManager.h"
#include "../Collider/ColliderModel.h"
#include "StageClassRoom.h"

StageClassRoom::StageClassRoom(void)
	:
	ActorBase()
{
}

StageClassRoom::~StageClassRoom(void)
{
}

void StageClassRoom::Update(void)
{
}

void StageClassRoom::Draw(void)
{
	ActorBase::Draw();
}

void StageClassRoom::Release(void)
{
	ActorBase::Release();
}

void StageClassRoom::InitLoad(void)
{

}

void StageClassRoom::InitTransform(void)
{
	transform_.scl = STAGE_DEFAULT_SCALE;
	transform_.quaRot = Quaternion::Identity();

	transform_.quaRotLocal = Quaternion::Identity();

	transform_.pos = STAGE_DEFAULT_POS;
	transform_.Update();
}

void StageClassRoom::InitCollider(void)
{
	// DxLib側の衝突情報セットアップ
	MV1SetupCollInfo(transform_.modelId);

	// モデルのコライダ
	ColliderModel* colModel =
		new ColliderModel(ColliderBase::TAG::STAGE, &transform_);

	//// 除外フレーム設定
	//for (const std::string& name : EXCLUDE_FRAME_NAMES)
	//{
	//	colModel->AddExcludeFrameIds(name);
	//}

	//// 許可フレーム設定
	//for (const std::string& name : TARGET_FRAME_NAMES)
	//{
	//	colModel->AddTargetFrameIds(name);
	//}


	ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::MODEL), colModel);
}

void StageClassRoom::InitAnimation(void)
{
}

void StageClassRoom::InitPost(void)
{
}
