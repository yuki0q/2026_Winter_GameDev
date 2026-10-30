#pragma once
#include <DxLib.h>
#include <map>
#include <string>
#include <vector>

// アイテムの種類ごとのパラメータ（ItemParam.csv）
struct ItemParam
{
    std::string name = "???";		// 表示名
    std::string description;		// 説明文
    bool storable = true;			// バッグに入れるか（false なら拾った瞬間に効果が出る）
    float scale = 1.0f;				// 大きさ（SpawnItem で位置だけ指定した時に使う）
    float pickupRange = 100.0f;		// 拾える距離（プレイヤーのカプセル半径に足す）
    int stackMax = 99;				// バッグの1枠に積める最大個数
    int value = 0;					// 効果量（回復薬ならHP回復量など。使い方は各アイテムのクラス次第）
};

// 配置1件分（ItemSpawn.csv）
struct ItemSpawnData
{
    std::string type;		// 「HP_POTION」など
    std::string timing;		// 出現タイミング（START: ゲーム開始時 / CLEAR: 敵を全滅させた時）
    VECTOR pos = { 0.0f, 0.0f, 0.0f };
};

// ドロップ1件分（ItemDrop.csv）
struct ItemDropData
{
    std::string enemy;		// 倒した敵の種類（「ALTER」など）
    std::string item;		// ドロップするアイテムの種類
    int rate = 100;			// ドロップ率（%）
};

// アイテム関連のCSVデータをまとめて管理する
class ItemDataTable
{

public:

    // 全CSVを読み込み直す（デバッグ中の再読み込みにも使用）
    static void Load(void);

    // アイテムの種類ごとのパラメータ（無ければ初期値）
    static ItemParam GetParam(const std::string& type);

    // 配置一覧
    static const std::vector<ItemSpawnData>& GetSpawns(void);

    // ドロップ一覧
    static const std::vector<ItemDropData>& GetDrops(void);

private:

    static void EnsureLoaded(void);
    static void LoadParam(void);
    static void LoadSpawn(void);
    static void LoadDrop(void);

    static inline bool isLoaded_ = false;
    static inline std::map<std::string, ItemParam> params_;
    static inline std::vector<ItemSpawnData> spawns_;
    static inline std::vector<ItemDropData> drops_;

};
