#include <Windows.h>
#include "../../../../Manager/CsvLoader.h"
#include "EnemyDataTable.h"

namespace
{
	// CSVの置き場所（実行時のカレントディレクトリ基準。既存の PATH_MODEL などに合わせて調整）
	const std::string PATH_CSV = "Data/CSV/";

	void LogWarning(const std::string& msg)
	{
		OutputDebugStringA((msg + "\n").c_str());
	}
}

void EnemyDataTable::Load(void)
{

	params_.clear();
	anims_.clear();
	spawns_.clear();

	LoadParam();
	LoadAnim();
	LoadSpawn();

	isLoaded_ = true;

}

EnemyParam EnemyDataTable::GetParam(const std::string& type)
{

	EnsureLoaded();

	auto it = params_.find(type);
	if (it == params_.end())
	{
		LogWarning("[EnemyDataTable] EnemyParam.csv に " + type + " がありません。初期値を使用します");
		return EnemyParam{};
	}

	return it->second;

}

const std::vector<EnemyAnimData>& EnemyDataTable::GetAnims(const std::string& type)
{

	EnsureLoaded();

	static const std::vector<EnemyAnimData> EMPTY;
	auto it = anims_.find(type);
	if (it == anims_.end())
	{
		LogWarning("[EnemyDataTable] EnemyAnim.csv に " + type + " がありません");
		return EMPTY;
	}

	return it->second;

}

const std::vector<EnemySpawnData>& EnemyDataTable::GetSpawns(void)
{
	EnsureLoaded();
	return spawns_;
}

void EnemyDataTable::EnsureLoaded(void)
{
	if (!isLoaded_)
	{
		Load();
	}
}

void EnemyDataTable::LoadParam(void)
{

	// type,hp,scale,walkSpeed,runSpeed,searchRadius,searchViewAngle,attackRadius,
	// attackColRadius,attackStartFrame,attackEndFrame,knockbackPower,knockbackTime,
	// lineTopY,capsuleTopY,capsuleDownY,capsuleRadius
	const size_t COLUMN_NUM = 17;

	auto rows = CsvLoader::Load(PATH_CSV + "EnemyParam.csv");
	if (rows.empty())
	{
		LogWarning("[EnemyDataTable] EnemyParam.csv を読み込めませんでした");
	}

	for (const auto& r : rows)
	{
		if (r.size() < COLUMN_NUM)
		{
			LogWarning("[EnemyDataTable] EnemyParam.csv の列数が足りない行があります: " + r[0]);
			continue;
		}

		EnemyParam p;
		p.hp = CsvLoader::ToInt(r[1], p.hp);
		p.scale = CsvLoader::ToFloat(r[2], p.scale);
		p.walkSpeed = CsvLoader::ToFloat(r[3], p.walkSpeed);
		p.runSpeed = CsvLoader::ToFloat(r[4], p.runSpeed);
		p.searchRadius = CsvLoader::ToFloat(r[5], p.searchRadius);
		p.searchViewAngle = CsvLoader::ToFloat(r[6], p.searchViewAngle);
		p.attackRadius = CsvLoader::ToFloat(r[7], p.attackRadius);
		p.attackColRadius = CsvLoader::ToFloat(r[8], p.attackColRadius);
		p.attackStartFrame = CsvLoader::ToInt(r[9], p.attackStartFrame);
		p.attackEndFrame = CsvLoader::ToInt(r[10], p.attackEndFrame);
		p.knockbackPower = CsvLoader::ToFloat(r[11], p.knockbackPower);
		p.knockbackTime = CsvLoader::ToFloat(r[12], p.knockbackTime);
		p.lineTopY = CsvLoader::ToFloat(r[13], p.lineTopY);
		p.capsuleTopY = CsvLoader::ToFloat(r[14], p.capsuleTopY);
		p.capsuleDownY = CsvLoader::ToFloat(r[15], p.capsuleDownY);
		p.capsuleRadius = CsvLoader::ToFloat(r[16], p.capsuleRadius);

		params_[r[0]] = p;
	}

}

void EnemyDataTable::LoadAnim(void)
{

	// owner,name,speed,path
	auto rows = CsvLoader::Load(PATH_CSV + "EnemyAnim.csv");
	if (rows.empty())
	{
		LogWarning("[EnemyDataTable] EnemyAnim.csv を読み込めませんでした");
	}

	for (const auto& r : rows)
	{
		if (r.size() < 4)
		{
			continue;
		}

		EnemyAnimData a;
		a.name = r[1];
		a.speed = CsvLoader::ToFloat(r[2], 30.0f);
		a.path = r[3];

		anims_[r[0]].push_back(a);
	}

}

void EnemyDataTable::LoadSpawn(void)
{

	// type,x,y,z
	auto rows = CsvLoader::Load(PATH_CSV + "EnemySpawn.csv");
	if (rows.empty())
	{
		LogWarning("[EnemyDataTable] EnemySpawn.csv を読み込めませんでした");
	}

	for (const auto& r : rows)
	{
		if (r.size() < 4)
		{
			continue;
		}

		EnemySpawnData s;
		s.type = r[0];
		s.pos = VGet(
			CsvLoader::ToFloat(r[1]),
			CsvLoader::ToFloat(r[2]),
			CsvLoader::ToFloat(r[3]));

		spawns_.push_back(s);
	}

}
