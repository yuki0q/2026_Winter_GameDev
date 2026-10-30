#pragma once
#include <vector>
#include "ItemBase.h"

// 拾ったアイテムを収納するバッグ（データのみ。操作や描画は ItemManager が行う）
// アイテムは「種類と個数」だけを持つ。使う時に ItemManager が一時的に生成して効果を発動する
class ItemBag
{
public:

    // 1枠分（同じ種類はまとめて個数で持つ）
    struct Slot
    {
        ItemBase::TYPE type;
        int count;
    };

     // 持てる種類の数（枠数）
    static constexpr int SLOT_MAX = 8;

    ItemBag(void);
    ~ItemBag(void);

    // 追加できるか（空き枠がある、または同じ種類で個数に余裕がある）
    bool CanAdd(ItemBase::TYPE type) const;

    // 1個追加（追加できなければ false）
    bool Add(ItemBase::TYPE type);

    // 指定枠から1個減らす（0個になったら枠ごと削除）
    bool RemoveOne(int slotIndex);

    // 空にする
    void Clear(void);

    const std::vector<Slot>& GetSlots(void) const { return slots_; }
    int GetSlotNum(void) const { return static_cast<int>(slots_.size()); }
    bool IsEmpty(void) const { return slots_.empty(); }

private:

    std::vector<Slot> slots_;

    // 同じ種類の枠を探す（無ければ -1）
    int FindSlot(ItemBase::TYPE type) const;
};
