#include "../../../Manager/InputManager.h"
#include "WeaponManager.h"

WeaponManager::WeaponManager(void)
    : 
    weaponModelId_(-1),
    isDebug_(false)
{
}

WeaponManager::~WeaponManager(void)
{
}

void WeaponManager::Init(int modelId)
{
    weaponModelId_ = modelId;
}

void WeaponManager::EquipWeapon(WEAPON_SLOT slot, std::shared_ptr<WeaponBase> weapon, 
    int modelHandle, int frameIndex,
    VECTOR scale, VECTOR localPos, VECTOR localRot)
{
    if (!weapon) return;

    weapon->Init(weaponModelId_, frameIndex, modelHandle, scale, localPos, localRot);
    weapons_[slot] = weapon;
}

void WeaponManager::UnequipWeapon(WEAPON_SLOT slot)
{
    auto it = weapons_.find(slot);
    if (it != weapons_.end())
    {
        if (it->second) it->second->Release();
        weapons_.erase(it);
    }
}

void WeaponManager::Update(void)
{
    for (auto& pair : weapons_)
    {
        if (pair.second)
        {
            pair.second->Update();
        }
    }
    UpdateDebug();
}

void WeaponManager::Draw(void)
{
    for (const auto& pair : weapons_)
    {
        if (pair.second)
        {
            pair.second->Draw();

            if (!isDebug_) continue;
            pair.second->DrawDebug();
        }
    }
}

void WeaponManager::Release(void)
{
    for (auto& pair : weapons_)
    {
        if (pair.second) pair.second->Release();
    }
    weapons_.clear();
}

void WeaponManager::SetAttackEnable(WEAPON_SLOT slot, bool isEnable)
{
    auto it = weapons_.find(slot);
    if (it != weapons_.end() && it->second)
    {
        it->second->SetAttackEnable(isEnable);
    }
}

const std::shared_ptr<WeaponBase>& WeaponManager::GetWeapon(WEAPON_SLOT slot) const
{
    auto it = weapons_.find(slot);
    if (it != weapons_.end())
    {
        return it->second;
    }

    // 戻り値は参照なので、一時オブジェクト(nullptr)ではなく、寿命の続く空のポインタを返す
    static const std::shared_ptr<WeaponBase> EMPTY;
    return EMPTY;
}

void WeaponManager::UpdateDebug(void)
{
    auto const& ins = InputManager::GetInstance();
    int debugkey = ins.IsTrgDown(KEY_INPUT_1);
    if (debugkey && !isDebug_)
    {
        isDebug_ = true;
    }
    else if (debugkey && isDebug_)
    {
        isDebug_ = false;
    }
}