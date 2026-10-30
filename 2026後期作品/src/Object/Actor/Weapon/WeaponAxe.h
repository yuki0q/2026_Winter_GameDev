#pragma once
#include "WeaponBase.h"

class WeaponAxe : public WeaponBase
{
public:
    WeaponAxe(void);
    ~WeaponAxe(void) override;

    void Init(int ownerModelId, int attachFrameIndex, int modelHandle,
        const VECTOR& scale, const VECTOR& localPos, const VECTOR& localRot) override;
};
