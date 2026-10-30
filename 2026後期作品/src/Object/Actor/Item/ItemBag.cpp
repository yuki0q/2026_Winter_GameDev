#include "ItemBag.h"

ItemBag::ItemBag(void)
{
}

ItemBag::~ItemBag(void)
{
}

bool ItemBag::CanAdd(ItemBase::TYPE type) const
{
    int index = FindSlot(type);
    if (index >= 0)
    {
        // “¯‚¶í—Ş‚Ì˜g‚ª‚ ‚ê‚ÎAŒÂ”‚É—]—T‚ª‚ ‚é‚©
        return slots_[index].count < 
            ItemDataTable::GetParam(ItemBase::TypeToName(type)).stackMax;
    }

    // V‚µ‚¢˜g‚ª•K—v‚È‚Ì‚ÅA‹ó‚«˜g‚ª‚ ‚é‚©
    return static_cast<int>(slots_.size()) < SLOT_MAX;
}

bool ItemBag::Add(ItemBase::TYPE type)
{
    if (!CanAdd(type))
    {
        return false;
    }

    int index = FindSlot(type);
    if (index >= 0)
    {
        slots_[index].count++;
    }
    else
    {
        slots_.push_back({ type, 1 });
    }

    return true;
}

bool ItemBag::RemoveOne(int slotIndex)
{
    if (slotIndex < 0 || slotIndex >= static_cast<int>(slots_.size()))
    {
        return false;
    }

    slots_[slotIndex].count--;
    if (slots_[slotIndex].count <= 0)
    {
        slots_.erase(slots_.begin() + slotIndex);
    }

    return true;
}

void ItemBag::Clear(void)
{
    slots_.clear();
}

int ItemBag::FindSlot(ItemBase::TYPE type) const
{
    for (int i = 0; i < static_cast<int>(slots_.size()); i++)
    {
        if (slots_[i].type == type)
        {
            return i;
        }
    }
    return -1;
}
