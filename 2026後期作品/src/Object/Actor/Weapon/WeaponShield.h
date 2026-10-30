#pragma once
#include "WeaponBase.h"

class WeaponShield : public WeaponBase
{
public:
    WeaponShield(void);
    ~WeaponShield(void) override;

    void Init(int ownerModelId, int attachFrameIndex, int modelHandle,
        const VECTOR& scale, const VECTOR& localPos, const VECTOR& localRot) override;
};
