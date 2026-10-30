#pragma once
#include <vector>
#include <memory>
#include <string>
#include <DxLib.h>
#include "../../../Object/Common/Transform.h"
#include "ItemDataTable.h"

class Player;
class ResourceManager;

class ItemBase
{
public:

    enum class TYPE
    {
        HP_POTION, // 回復薬
        JEWEL,
        GOLD,      // コイン・スコア等
    };

    ItemBase(TYPE type, const VECTOR& pos, const VECTOR& scl, const VECTOR& rot);
    virtual ~ItemBase(void);

    virtual void Init(void);
    virtual void Update(void);
    virtual void Draw(void);
    virtual void Release(void);

    // 取得済みか
    bool IsCollected(void) const { return isCollected_; }
    void SetCollected(bool collected) { isCollected_ = collected; }

    TYPE GetItemType(void)const { return type_; }

    // アイテム種別 <-> CSV上の名前
    static std::string TypeToName(TYPE type);
    static bool NameToType(const std::string& name, TYPE& outType);

    // CSV(item_param.csv)から読み込んだパラメータ
    const ItemParam& GetParam(void) const { return param_; }

    // バッグに収納できるか（false の場合は拾った瞬間に効果が発動する）
    virtual bool IsStorable(void) const { return param_.storable; }

    const Transform& GetTransform(void) const { return transform_; }

    // プレイヤーが取得した時の効果
    virtual void OnApplyEffect(std::shared_ptr<Player> player) = 0;

protected:

    // 効果量(value)などは、ここから参照する
    ItemParam param_;

    TYPE type_;
    Transform transform_;
    bool isCollected_;
    ResourceManager& resMng_;
};