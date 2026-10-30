#pragma once
#include <DxLib.h>
#include <memory>
#include <vector>
#include <string>
#include "../../Common/Transform.h"
#include "../../Collider/ColliderBase.h"
#include "../Charactor/PlayerDataTable.h"

enum class WEAPON_TYPE
{
    NONE,
    SWORD,
    SHIELD,
    GREAT_SWORD,
    AXE
};

// 武器種別 <-> CSV上の名前
std::string WeaponTypeToName(WEAPON_TYPE type);
WEAPON_TYPE NameToWeaponType(const std::string& name);

class WeaponBase
{
public:
    WeaponBase(void);
    virtual ~WeaponBase(void);

    virtual void Init(int ownerModelId, int attachFrameIndex, int modelHandle,
        const VECTOR& scale, const VECTOR& localPos, const VECTOR& localRot);
    virtual void Update(void);
    virtual void Draw(void);
    virtual void DrawDebug(void);
    virtual void Release(void);

    // 攻撃判定の有効・無効フラグ
    void SetAttackEnable(bool isEnable) { isAttackEnable_ = isEnable; }
    bool IsAttackEnable(void) const { return isAttackEnable_; }

    // 衝突判定（コライダ）の取得
    const ColliderBase* GetCollider(void) const { return collider_.get(); }
    const WEAPON_TYPE& GetType(void) const { return type_; }

    // ダメージ量（weapon_param.csv）
    int GetDamage(void) const { return param_.damage; }

protected:

    // weapon_param.csv から読み込んだパラメータ（Init で type_ に対応する行を取得）
    WeaponParam param_;

    WEAPON_TYPE type_;
    int weaponModelId_;
    int attachFrameIndex_;
    int modelHandle_;

    // 武器自身のTransform（ワールド座標同期用）
    Transform transform_;           
    VECTOR localPos_;
    VECTOR localRot_;
    VECTOR scale_;
    // 攻撃判定中かどうか
    bool isAttackEnable_;

    // 武器のコライダ
    std::unique_ptr<ColliderBase> collider_; 
};