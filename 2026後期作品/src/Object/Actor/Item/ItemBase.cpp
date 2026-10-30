#include "ItemBase.h"
#include "../Charactor/Player.h"
#include "../../../Manager/ResourceManager.h"

ItemBase::ItemBase(TYPE type, const VECTOR& pos, const VECTOR& scl, const VECTOR& rot)
    : type_(type), 
    isCollected_(false), 
    resMng_(ResourceManager::GetInstance())
{
    // 種類に対応するパラメータをCSVから取得
    param_ = ItemDataTable::GetParam(TypeToName(type));

    transform_.pos = pos;
    transform_.scl = scl;
    transform_.quaRot = rot;
    transform_.Update();
}

std::string ItemBase::TypeToName(TYPE type)
{
    switch (type)
    {
    case TYPE::HP_POTION: return "HP_POTION";
    case TYPE::GOLD:      return "GOLD";
    case TYPE::JEWEL:     return "JEWEL";
    default:              return "";
    }
}

bool ItemBase::NameToType(const std::string& name, TYPE& outType)
{
    // 新しいアイテムを増やす時は、ここにも追加する
    static const TYPE ALL_TYPES[] = 
    { TYPE::HP_POTION, TYPE::GOLD, TYPE::JEWEL };

    for (TYPE type : ALL_TYPES)
    {
        if (TypeToName(type) == name)
        {
            outType = type;
            return true;
        }
    }
    return false;
}

ItemBase::~ItemBase(void) 
{
}

void ItemBase::Init() 
{
}

void ItemBase::Update() 
{ 
    transform_.Update(); 
}

void ItemBase::Draw()
{
    if (isCollected_) return;

    if (transform_.modelId != -1)
    {
        MV1DrawModel(transform_.modelId);
    }
}

void ItemBase::Release()
{
    if (transform_.modelId != -1)
    {
        MV1DeleteModel(transform_.modelId);
    }
}