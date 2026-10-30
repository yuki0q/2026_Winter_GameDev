#include "WeaponSword.h"
#include "../../Collider/ColliderCapsule.h"

WeaponSword::WeaponSword(void)
{
    type_ = WEAPON_TYPE::SWORD;
}

WeaponSword::~WeaponSword(void)
{
}

void WeaponSword::Init(int ownerModelId, int attachFrameIndex, int modelHandle,
    const VECTOR& scale, const VECTOR& localPos, const VECTOR& localRot)
{
    WeaponBase::Init(ownerModelId, attachFrameIndex, modelHandle, scale, localPos, localRot);

    // 刀身に沿ったカプセル範囲（柄から剣先）を設定
    // 武器モデルのローカル軸に合わせて調整してください
    VECTOR localTop = { 0.0f, param_.colTopY, 0.0f };   // 柄の位置
    VECTOR localDown = { 0.0f, param_.colDownY, 0.0f }; // 剣先方向へのオフセット
    float radius = param_.colRadius;                    // 当たり判定の太さ

    // transform_ を参照先としてカプセルコライダを生成
    collider_ = std::make_unique<ColliderCapsule>(
        ColliderBase::TAG::WEAPON, &transform_, localTop, localDown, radius);
}