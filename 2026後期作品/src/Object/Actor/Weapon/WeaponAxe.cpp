#include "WeaponAxe.h"
#include "../../Collider/ColliderCapsule.h"

WeaponAxe::WeaponAxe(void)
{
    type_ = WEAPON_TYPE::AXE;
}

WeaponAxe::~WeaponAxe(void)
{
}

void WeaponAxe::Init(int ownerModelId, int attachFrameIndex, int modelHandle,
    const VECTOR& scale, const VECTOR& localPos, const VECTOR& localRot)
{
    WeaponBase::Init(ownerModelId, attachFrameIndex, modelHandle, scale, localPos, localRot);

    // 斧の柄から刃先までを覆うカプセル範囲を weapon_param.csv の値で設定
    // 武器モデルのローカル軸に合わせて、デバッグ表示（1キー）を見ながら調整してください
    VECTOR localTop = { 0.0f, param_.colTopY, 0.0f };    // 柄側
    VECTOR localDown = { 0.0f, param_.colDownY, 0.0f };  // 刃先側
    float radius = param_.colRadius;

    // transform_ を参照先としてカプセルコライダを生成
    collider_ = std::make_unique<ColliderCapsule>(
        ColliderBase::TAG::WEAPON, &transform_, localTop, localDown, radius);
}
