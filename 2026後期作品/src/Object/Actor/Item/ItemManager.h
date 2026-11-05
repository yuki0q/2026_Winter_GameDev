#pragma once
#include <vector>
#include <memory>
#include <string>
#include <DxLib.h>
#include "ItemBase.h"
#include "ItemBag.h"

class Player;

class ItemManager
{
public:

    ItemManager(void);
    ~ItemManager(void);

    void Init(void);
    void Update(void);
    void Draw(void);
    void Release(void);

    // アイテムの生成
    void SpawnItem(ItemBase::TYPE type, const VECTOR& pos, const VECTOR& scl, const VECTOR& rot);

    // アイテムの生成（大きさは item_param.csv の scale、回転なし）
    void SpawnItem(ItemBase::TYPE type, const VECTOR& pos);

    // item_spawn.csv のうち、指定したタイミング（START / CLEAR など）の行をまとめて生成
    void SpawnFromCsv(const std::string& timing);

    // 敵を倒した時のドロップ（item_drop.csv の確率に従って、倒した位置の少し上に生成）
    void DropFromEnemy(const std::string& enemyName, const VECTOR& enemyPos);

    // 拾える範囲にあるアイテムの判定と、Fキーでの取得処理
    void CheckCollection(std::shared_ptr<Player> player);

    // 拾える対象の上に「[F] 拾う」を描画（3D描画の後、2D描画フェーズで呼ぶ）
    void DrawPickupPrompt(void) const;

    // 拾える範囲にアイテムがあるか
    bool HasPickableItem(void) const { return pickableItem_ != nullptr; }

    // バッグの開閉・操作（毎フレーム呼ぶ）
    // Bキーで開閉し、開いている間は W/S(↑↓) で選択、E(Enter,Space) で使う、ESC で閉じる
    void UpdateBag(std::shared_ptr<Player> player);

    // バッグの描画（2D描画フェーズの最後に呼ぶ）
    void DrawBag(void) const;

    // バッグを開いているか（開いている間はゲーム内の更新を止める想定）
    bool IsBagOpen(void) const { return isBagOpen_; }

    const ItemBag& GetBag(void) const { return bag_; }

    // ItemGold(COIN)を取得したか
    bool IsGoldCollected(void) const { return isGoldCollected_; }

private:

    std::vector<std::shared_ptr<ItemBase>> items_;

    // 今フレームで拾える範囲にある、最も近いアイテム（無ければ nullptr）
    std::shared_ptr<ItemBase> pickableItem_;

    // 収納用のバッグ
    ItemBag bag_;
    bool isBagOpen_ = false;
    // バッグ内で選択中の枠
    int bagCursor_ = 0;

    // アイテムの生成（Init は呼ばない）
    std::shared_ptr<ItemBase> CreateItem(ItemBase::TYPE type,
        const VECTOR& pos, const VECTOR& scl, const VECTOR& rot) const;

    // バッグ内の指定枠のアイテムを使う
    void UseBagItem(std::shared_ptr<Player> player, int slotIndex);

    // ItemGold を取得したか（取得済みアイテムはリストから消えるため別に保持）
    bool isGoldCollected_ = false;

    // スティック
    bool isStickUpOld;
    bool isStickDownOld;
};

