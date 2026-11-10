#include <Windows.h>
#include <set>
#include "../../../Manager/CsvLoader.h"
#include "ItemDataTable.h"

namespace
{
    // CSVの置き場所（EnemyDataTable / PlayerDataTable と同じ値にする）
    const std::string PATH_CSV = "Data/CSV/";

    void LogWarning(const std::string& msg)
    {
        // 同じ警告は1回だけ出力する
        // （毎フレーム呼ばれる場所で表に無い項目を引いても、出力が溜まって重くならないようにする）
        static std::set<std::string> logged;
        if (!logged.insert(msg).second)
        {
            return;
        }

        OutputDebugStringA((msg + "\n").c_str());
    }
}

void ItemDataTable::Load(void)
{
    params_.clear();
    spawns_.clear();
    drops_.clear();

    LoadParam();
    LoadSpawn();
    LoadDrop();

    isLoaded_ = true;
}

ItemParam ItemDataTable::GetParam(const std::string& type)
{
    EnsureLoaded();

    auto it = params_.find(type);
    if (it == params_.end())
    {
        LogWarning("[ItemDataTable] ItemParam.csv に " + type + " がありません。初期値を使用します");
        return ItemParam{};
    }

    return it->second;
}

const std::vector<ItemSpawnData>& ItemDataTable::GetSpawns(void)
{
    EnsureLoaded();
    return spawns_;
}

const std::vector<ItemDropData>& ItemDataTable::GetDrops(void)
{
    EnsureLoaded();
    return drops_;
}

void ItemDataTable::EnsureLoaded(void)
{
    if (!isLoaded_)
    {
        Load();
    }
}

void ItemDataTable::LoadParam(void)
{
    // type,name,description,storable,scale,pickupRange,stackMax,value
    const size_t COLUMN_NUM = 8;

    auto rows = CsvLoader::Load(PATH_CSV + "ItemParam.csv");
    if (rows.empty())
    {
        LogWarning("[ItemDataTable] ItemParam.csv を読み込めませんでした");
    }

    for (const auto& r : rows)
    {
        if (r.size() < COLUMN_NUM)
        {
            LogWarning("[ItemDataTable] ItemParam.csv の列数が足りない行があります: " + r[0]);
            continue;
        }

        ItemParam p;
        p.name = r[1];
        p.description = r[2];
        p.storable = CsvLoader::ToInt(r[3], p.storable ? 1 : 0) != 0;
        p.scale = CsvLoader::ToFloat(r[4], p.scale);
        p.pickupRange = CsvLoader::ToFloat(r[5], p.pickupRange);
        p.stackMax = CsvLoader::ToInt(r[6], p.stackMax);
        p.value = CsvLoader::ToInt(r[7], p.value);

        params_[r[0]] = p;
    }
}

void ItemDataTable::LoadSpawn(void)
{
    // type,timing,x,y,z
    auto rows = CsvLoader::Load(PATH_CSV + "ItemSpawn.csv");
    if (rows.empty())
    {
        LogWarning("[ItemDataTable] ItemSpawn.csv を読み込めませんでした");
    }

    for (const auto& r : rows)
    {
        if (r.size() < 5)
        {
            continue;
        }

        ItemSpawnData s;
        s.type = r[0];
        s.timing = r[1];
        s.pos = VGet(
            CsvLoader::ToFloat(r[2]),
            CsvLoader::ToFloat(r[3]),
            CsvLoader::ToFloat(r[4]));

        spawns_.push_back(s);
    }
}

void ItemDataTable::LoadDrop(void)
{
    // enemy,item,rate
    auto rows = CsvLoader::Load(PATH_CSV + "ItemDrop.csv");
    if (rows.empty())
    {
        LogWarning("[ItemDataTable] ItemDrop.csv を読み込めませんでした");
    }

    for (const auto& r : rows)
    {
        if (r.size() < 3)
        {
            continue;
        }

        ItemDropData d;
        d.enemy = r[0];
        d.item = r[1];
        d.rate = CsvLoader::ToInt(r[2], 100);

        drops_.push_back(d);
    }
}
