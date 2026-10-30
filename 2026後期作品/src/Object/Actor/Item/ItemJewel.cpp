#include "ItemJewel.h"
#include "../../../Manager/ResourceManager.h"
#include "../Charactor/Player.h"

ItemJewel::ItemJewel(const VECTOR& pos, const VECTOR& scl, const VECTOR& rot)
    : ItemBase(TYPE::JEWEL, pos, scl, rot)
{
}

void ItemJewel::Init()
{
    // 必要に応じてモデルのロードなどを記述
    transform_.modelId = resMng_.LoadModelDuplicate(
        ResourceManager::SRC::MDL_POTION);
}

void ItemJewel::OnApplyEffect(std::shared_ptr<Player> player)
{
    if (!player) return;
    // プレイヤーのHP回復処理など
    // player->AddHP(30.0f);
    player->AddHP(30);
}