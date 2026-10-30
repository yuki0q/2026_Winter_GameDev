#pragma once
#include <DxLib.h>
#include <map>
#include <string>
#include <memory>
#include "../../Common/Transform.h"
#include "WeaponBase.h"

// 装着スロットの定義
enum class WEAPON_SLOT
{
    RIGHT_HAND, // 右手
    LEFT_HAND,  // 左手
    BACK,       // 背中（納刀時）
    MAX
};

class WeaponManager
{
public:

    WeaponManager(void);
    ~WeaponManager(void);

    // 初期化（所有者のモデルIDを登録）
    void Init(int ownerModelId);

    // 武器の登録・アタッチ
    void EquipWeapon(WEAPON_SLOT slot, std::shared_ptr<WeaponBase> weapon, int modelHandle, int frameIndex,
        VECTOR scale,VECTOR localPos,VECTOR localRot);

    // 武器の取り外し
    void UnequipWeapon(WEAPON_SLOT slot);

    // 更新（キャラクターのフレーム行列に合わせて武器のTransformを計算）
    void Update(void);
    // 描画
    void Draw(void);
    // 解放
    void Release(void);    
    
    void SetAttackEnable(WEAPON_SLOT slot, bool isEnable);
    const std::shared_ptr<WeaponBase>& GetWeapon(WEAPON_SLOT slot) const;

private:

    int weaponModelId_;
    std::map<WEAPON_SLOT, std::shared_ptr<WeaponBase>> weapons_;

    void UpdateDebug(void);
    bool isDebug_;
};

