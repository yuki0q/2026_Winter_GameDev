#pragma once
#include "ItemBase.h"

class ItemHpPotion : public ItemBase
{
public:

    ItemHpPotion(const VECTOR& pos, const VECTOR& scl, const VECTOR& rot);

    void Init() override;
    void OnApplyEffect(std::shared_ptr<Player> player) override;
};