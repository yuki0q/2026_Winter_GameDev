#pragma once
#include "WeaponBase.h"

class WeaponSword : public WeaponBase
{
public:
    WeaponSword(void);
    ~WeaponSword(void) override;

    void Init(int ownerModelId, int attachFrameIndex, int modelHandle,
        const VECTOR& scale, const VECTOR& localPos, const VECTOR& localRot) override;
};

