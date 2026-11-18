#pragma once
#include "WeaponBase.h"

class WeaponGreatSword : public WeaponBase
{
public:
    WeaponGreatSword(void);
    ~WeaponGreatSword(void) override;

    void Init(int ownerModelId, int attachFrameIndex, int modelHandle,
        const VECTOR& scale, const VECTOR& localPos, const VECTOR& localRot) override;
};
