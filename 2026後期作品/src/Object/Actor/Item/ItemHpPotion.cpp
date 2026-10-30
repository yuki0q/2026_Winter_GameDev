#include "ItemHpPotion.h"
#include "../../../Manager/ResourceManager.h"
#include "../Charactor/Player.h"

ItemHpPotion::ItemHpPotion(const VECTOR& pos, const VECTOR& scl, const VECTOR& rot)
    : ItemBase(TYPE::HP_POTION, pos ,scl, rot)
{
}

void ItemHpPotion::Init()
{
    // 必要に応じてモデルのロードなどを記述
    transform_.modelId = resMng_.LoadModelDuplicate(
        ResourceManager::SRC::MDL_POTION);
}

void ItemHpPotion::OnApplyEffect(std::shared_ptr<Player> player)
{
    if (!player) return;
    // プレイヤーのHP回復処理など
    // player->AddHP(30.0f);
    player->AddHP(30);
}