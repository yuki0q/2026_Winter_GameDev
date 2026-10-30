#include <DxLib.h>
#include "ItemGold.h"
#include "../../../Manager/ResourceManager.h"
#include "../../../Utility/AsoUtility.h"

namespace
{
    // 回転速度（ラジアン/フレーム）
    const float SPIN_SPEED_RAD = 0.03f;

    // モデルが読み込めなかった時に表示する球の半径（スケール1.0のとき）
    const float FALLBACK_RADIUS = 30.0f;
}

ItemGold::ItemGold(const VECTOR& pos, const VECTOR& scl, const VECTOR& rot)
    : ItemBase(TYPE::GOLD, pos, scl, rot)
{
}

ItemGold::~ItemGold(void)
{
}

void ItemGold::Init()
{
    // モデルを複製して読み込む（読み込めなかった場合は Draw で球を表示する）
    int modelId = resMng_.LoadModelDuplicate(ResourceManager::SRC::MDL_GOLD);
    if (modelId != -1)
    {
        transform_.SetModel(modelId);
    }
    transform_.Update();
}

void ItemGold::Update(void)
{
    // 取得しやすいように、Y軸でゆっくり回転させて目立たせる
    transform_.quaRot = Quaternion::Mult(transform_.quaRot,
        Quaternion::AngleAxis(SPIN_SPEED_RAD, AsoUtility::AXIS_Y));

    ItemBase::Update();
}

void ItemGold::Draw(void)
{
    if (isCollected_) return;

    if (transform_.modelId != -1)
    {
        ItemBase::Draw();
        return;
    }

    // モデルが無い場合の代替表示（金色の球）
    DrawSphere3D(transform_.pos, FALLBACK_RADIUS * transform_.scl.x, 16,
        GetColor(255, 215, 0), GetColor(255, 255, 255), TRUE);
}

void ItemGold::OnApplyEffect(std::shared_ptr<Player> player)
{
    // 取得したこと自体は ItemManager::IsGoldCollected() で GameScene に伝わる
    // スコア加算などを入れる場合はここに書く
}
