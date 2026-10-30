#include "ItemManager.h"
#include <algorithm>
#include <cfloat>
#include <cstdio>
#include <cstring>
#include "../../../Manager/InputManager.h"
#include "../Charactor/Player.h"
#include "../../Collider/ColliderCapsule.h"
#include "ItemHpPotion.h"
#include "ItemGold.h"
#include "ItemJewel.h"

namespace
{
    // ドロップしたアイテムを出す高さ（敵の足元から）
    const float DROP_OFFSET_Y = 60.0f;

    // 拾う操作のキー
    const int PICKUP_KEY = KEY_INPUT_F;

    // プロンプトをアイテムの何ユニット上に出すか
    const float PROMPT_OFFSET_Y = 100.0f;

    // プロンプトの背景の余白
    const int PROMPT_PADDING = 8;

    // バッグの操作キー
    const int BAG_KEY = KEY_INPUT_B;

    // バッグ画面のレイアウト
    const int BAG_PANEL_W = 560;
    const int BAG_PANEL_H = 440;
    const int BAG_ROW_H = 36;
    const int BAG_ROW_MARGIN = 24;
    const int BAG_TITLE_FONT_SIZE = 28;
    const int BAG_TEXT_FONT_SIZE = 22;
    const int BAG_HINT_FONT_SIZE = 18;
}

ItemManager::ItemManager(void) 
{
}

ItemManager::~ItemManager(void) 
{
}

void ItemManager::Init(void) 
{
}

void ItemManager::Update(void)
{
    for (auto& item : items_)
    {
        if (item && !item->IsCollected())
        {
            item->Update();
        }
    }
}

void ItemManager::Draw(void)
{
    for (auto& item : items_)
    {
        if (item && !item->IsCollected())
        {
            item->Draw();
        }
    }
}

void ItemManager::Release(void)
{
    for (auto& item : items_)
    {
        if (item) item->Release();
    }
    items_.clear();
    pickableItem_ = nullptr;
    bag_.Clear();
    isBagOpen_ = false;
    bagCursor_ = 0;
}

std::shared_ptr<ItemBase> ItemManager::CreateItem(ItemBase::TYPE type,
    const VECTOR& pos, const VECTOR& scl, const VECTOR& rot) const
{
    if (type == ItemBase::TYPE::HP_POTION)
    {
        return std::make_shared<ItemHpPotion>(pos, scl, rot);
    }
    else if (type == ItemBase::TYPE::GOLD)
    {
        return std::make_shared<ItemGold>(pos, scl, rot);
    }
    else if (type == ItemBase::TYPE::JEWEL)
    {
        return std::make_shared<ItemJewel>(pos, scl, rot);
    }

    return nullptr;
}

void ItemManager::SpawnItem(ItemBase::TYPE type, const VECTOR& pos, const VECTOR& scl, const VECTOR& rot)
{
    std::shared_ptr<ItemBase> newItem = CreateItem(type, pos, scl, rot);

    if (newItem)
    {
        newItem->Init();
        items_.push_back(newItem);
    }
}

void ItemManager::SpawnItem(ItemBase::TYPE type, const VECTOR& pos)
{
    const ItemParam param = ItemDataTable::GetParam(ItemBase::TypeToName(type));
    SpawnItem(type, pos,
        VGet(param.scale, param.scale, param.scale), VGet(0.0f, 0.0f, 0.0f));
}

void ItemManager::SpawnFromCsv(const std::string& timing)
{
    for (const auto& spawn : ItemDataTable::GetSpawns())
    {
        if (spawn.timing != timing) continue;

        ItemBase::TYPE type;
        if (!ItemBase::NameToType(spawn.type, type))
        {
            // 未知のアイテム種別はスキップ
            continue;
        }
        SpawnItem(type, spawn.pos);
    }
}

void ItemManager::DropFromEnemy(const std::string& enemyName, const VECTOR& enemyPos)
{
    for (const auto& drop : ItemDataTable::GetDrops())
    {
        if (drop.enemy != enemyName) continue;

        // ドロップ率(%)の判定（100 なら必ずドロップ、0 なら出ない）
        if (GetRand(99) >= drop.rate) continue;

        ItemBase::TYPE type;
        if (!ItemBase::NameToType(drop.item, type)) continue;

        SpawnItem(type, VAdd(enemyPos, VGet(0.0f, DROP_OFFSET_Y, 0.0f)));
    }
}

void ItemManager::CheckCollection(std::shared_ptr<Player> player)
{
    // 拾える対象は毎フレーム探し直す
    pickableItem_ = nullptr;

    if (!player) return;

    // プレイヤーのカプセルコライダを取得
    const auto* playerCapsule = dynamic_cast<const ColliderCapsule*>(
        player->GetOwnCollider(static_cast<int>(CharactorBase::COLLIDER_TYPE::CAPSULE))
        );
    if (!playerCapsule) return;

    VECTOR playerCenter = playerCapsule->GetCenter();
    float playerRadius = playerCapsule->GetRadius();

    // プレイヤーと各アイテムの距離をチェック
    float nearestDistSq = FLT_MAX;
    for (auto& item : items_)
    {
        if (!item || item->IsCollected()) continue;

        VECTOR diff = VSub(playerCenter, item->GetTransform().pos);
        float distSq = VDot(diff, diff);

        // 拾える範囲は、アイテムごとの pickupRange（item_param.csv）
        float pickRange = playerRadius + item->GetParam().pickupRange;

        if (distSq <= pickRange * pickRange && distSq < nearestDistSq)
        {
            nearestDistSq = distSq;
            pickableItem_ = item;
        }
    }

    // 拾える物が無ければ終了
    if (!pickableItem_) return;

    const bool isStorable = pickableItem_->IsStorable();

    // バッグに入れる物は、空きが無ければ拾えない（プロンプトは「バッグがいっぱい」になる）
    if (isStorable && !bag_.CanAdd(pickableItem_->GetItemType())) return;

    // Fキーが押された時だけ拾う
    auto const& ins = InputManager::GetInstance();
    if (!ins.IsTrgDown(PICKUP_KEY)) return;

    if (isStorable)
    {
        // バッグに収納（使う時に効果を発動する）
        bag_.Add(pickableItem_->GetItemType());
    }
    else
    {
        // 収納しないアイテムは、拾った瞬間に効果を発動
        pickableItem_->OnApplyEffect(player);

        // ItemGold を取った事を記録（GameScene がクリア遷移に使う）
        if (pickableItem_->GetItemType() == ItemBase::TYPE::GOLD)
        {
            isGoldCollected_ = true;
        }
    }

    // ワールド上のアイテムは取得済みにして消す
    pickableItem_->SetCollected(true);
    pickableItem_ = nullptr;

    // 取得済みのアイテムをリストから安全に削除
    items_.erase(
        std::remove_if(items_.begin(), items_.end(), [](const std::shared_ptr<ItemBase>& item) {
            if (!item || item->IsCollected())
            {
                if (item) item->Release();
                return true;
            }
            return false;
            }),
        items_.end()
    );
}

void ItemManager::DrawPickupPrompt(void) const
{
    if (!pickableItem_) return;

    // アイテムの少し上のワールド座標を、画面座標へ変換
    VECTOR worldPos = VAdd(pickableItem_->GetTransform().pos, VGet(0.0f, PROMPT_OFFSET_Y, 0.0f));
    VECTOR screenPos = ConvWorldPosToScreenPos(worldPos);

    // カメラの後ろや描画範囲外なら表示しない
    if (screenPos.z <= 0.0f || screenPos.z >= 1.0f) return;

    // バッグがいっぱいで拾えない場合は、その旨を表示
    const bool isFull = pickableItem_->IsStorable() &&
        !bag_.CanAdd(pickableItem_->GetItemType());
    const char* text = isFull ? "バッグがいっぱい" : "[F] 拾う";

    int width = GetDrawStringWidth(text, static_cast<int>(std::strlen(text)));
    int height = GetFontSize();

    int x = static_cast<int>(screenPos.x) - width / 2;
    int y = static_cast<int>(screenPos.y) - height / 2;

    // 半透明の背景
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 160);
    DrawBox(x - PROMPT_PADDING, y - PROMPT_PADDING / 2,
        x + width + PROMPT_PADDING, y + height + PROMPT_PADDING / 2,
        GetColor(0, 0, 0), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    DrawString(x, y, text, isFull ? GetColor(255, 120, 120) : GetColor(255, 255, 255));
}

void ItemManager::UpdateBag(std::shared_ptr<Player> player)
{
    auto const& ins = InputManager::GetInstance();

    // 開閉
    if (ins.IsTrgDown(BAG_KEY))
    {
        isBagOpen_ = !isBagOpen_;

        // 開く時は、カーソルを範囲内に収める
        int maxIndex = bag_.GetSlotNum() - 1;
        if (bagCursor_ > maxIndex)
        {
            bagCursor_ = maxIndex > 0 ? maxIndex : 0;
        }
        return;
    }

    if (!isBagOpen_) return;

    // ESC でも閉じる（GameScene 側では、バッグ操作中のフレームはポーズ判定をしない）
    if (ins.IsTrgDown(KEY_INPUT_ESCAPE))
    {
        isBagOpen_ = false;
        return;
    }

    const int slotNum = bag_.GetSlotNum();
    if (slotNum <= 0) return;

    // カーソル移動（上下でループ）
    if (ins.IsTrgDown(KEY_INPUT_UP) || ins.IsTrgDown(KEY_INPUT_W))
    {
        bagCursor_ = (bagCursor_ - 1 + slotNum) % slotNum;
    }
    if (ins.IsTrgDown(KEY_INPUT_DOWN) || ins.IsTrgDown(KEY_INPUT_S))
    {
        bagCursor_ = (bagCursor_ + 1) % slotNum;
    }

    // 使う
    if (ins.IsTrgDown(PICKUP_KEY) || ins.IsTrgDown(KEY_INPUT_RETURN) || ins.IsTrgDown(KEY_INPUT_SPACE))
    {
        UseBagItem(player, bagCursor_);
    }
}

void ItemManager::UseBagItem(std::shared_ptr<Player> player, int slotIndex)
{
    if (!player) return;

    const auto& slots = bag_.GetSlots();
    if (slotIndex < 0 || slotIndex >= static_cast<int>(slots.size())) return;

    // 効果を発動するために、その種類のアイテムを一時的に生成する（モデルは読み込まない）
    auto item = CreateItem(slots[slotIndex].type,
        VGet(0.0f, 0.0f, 0.0f), VGet(1.0f, 1.0f, 1.0f), VGet(0.0f, 0.0f, 0.0f));
    if (!item) return;

    item->OnApplyEffect(player);

    // 1個消費（0個になったら枠ごと消える）
    bag_.RemoveOne(slotIndex);

    // 枠が減った場合に、カーソルが範囲外にならないようにする
    int maxIndex = bag_.GetSlotNum() - 1;
    if (bagCursor_ > maxIndex)
    {
        bagCursor_ = maxIndex > 0 ? maxIndex : 0;
    }
}

void ItemManager::DrawBag(void) const
{
    if (!isBagOpen_) return;

    int sx, sy;
    GetDrawScreenSize(&sx, &sy);

    const int prevFontSize = GetFontSize();

    // 背景を暗くする
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 150);
    DrawBox(0, 0, sx, sy, GetColor(0, 0, 0), TRUE);

    // パネル
    const int px = (sx - BAG_PANEL_W) / 2;
    const int py = (sy - BAG_PANEL_H) / 2;
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 235);
    DrawBox(px, py, px + BAG_PANEL_W, py + BAG_PANEL_H, GetColor(30, 30, 45), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    DrawBox(px, py, px + BAG_PANEL_W, py + BAG_PANEL_H, GetColor(200, 200, 220), FALSE);

    // タイトル
    SetFontSize(BAG_TITLE_FONT_SIZE);
    DrawFormatString(px + BAG_ROW_MARGIN, py + 16, GetColor(255, 255, 255),
        "バッグ  %d / %d", bag_.GetSlotNum(), ItemBag::SLOT_MAX);

    // アイテム一覧
    SetFontSize(BAG_TEXT_FONT_SIZE);
    const int listTop = py + 70;
    const auto& slots = bag_.GetSlots();

    if (slots.empty())
    {
        DrawString(px + BAG_ROW_MARGIN, listTop + 8, "アイテムを持っていません",
            GetColor(180, 180, 180));
    }

    for (int i = 0; i < static_cast<int>(slots.size()); i++)
    {
        const int rowY = listTop + i * BAG_ROW_H;
        const int textY = rowY + (BAG_ROW_H - BAG_TEXT_FONT_SIZE) / 2;
        const bool isSelected = (i == bagCursor_);

        // 選択中の行を強調
        if (isSelected)
        {
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, 90);
            DrawBox(px + 8, rowY, px + BAG_PANEL_W - 8, rowY + BAG_ROW_H - 2,
                GetColor(255, 220, 80), TRUE);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
            DrawString(px + 12, textY, ">", GetColor(255, 220, 80));
        }

        unsigned int color = isSelected ? GetColor(255, 255, 255) : GetColor(200, 200, 200);

        // 名前
        const ItemParam info = ItemDataTable::GetParam(ItemBase::TypeToName(slots[i].type));
        DrawString(px + BAG_ROW_MARGIN + 12, textY, info.name.c_str(), color);

        // 個数（右寄せ）
        char countText[16];
        std::snprintf(countText, sizeof(countText), "x %d", slots[i].count);
        int countW = GetDrawStringWidth(countText, static_cast<int>(std::strlen(countText)));
        DrawString(px + BAG_PANEL_W - BAG_ROW_MARGIN - countW, textY, countText, color);
    }

    // 選択中のアイテムの説明
    if (!slots.empty() && bagCursor_ >= 0 && bagCursor_ < static_cast<int>(slots.size()))
    {
        const int descY = py + BAG_PANEL_H - 78;
        DrawLine(px + 8, descY - 8, px + BAG_PANEL_W - 8, descY - 8, GetColor(120, 120, 140));
        const ItemParam info = ItemDataTable::GetParam(
            ItemBase::TypeToName(slots[bagCursor_].type));
        DrawString(px + BAG_ROW_MARGIN, descY, info.description.c_str(),
            GetColor(220, 220, 220));
    }

    // 操作説明
    SetFontSize(BAG_HINT_FONT_SIZE);
    DrawString(px + BAG_ROW_MARGIN, py + BAG_PANEL_H - 34,
        "W/S:選択   E/Space:使う   B/ESC:閉じる", GetColor(160, 160, 180));

    SetFontSize(prevFontSize);
}