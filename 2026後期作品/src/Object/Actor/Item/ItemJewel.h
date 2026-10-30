#pragma once
#include "ItemBase.h"

class ItemJewel : public ItemBase
{
public:

    ItemJewel(const VECTOR& pos, const VECTOR& scl, const VECTOR& rot);

    void Init() override;
    void OnApplyEffect(std::shared_ptr<Player> player) override;
};

