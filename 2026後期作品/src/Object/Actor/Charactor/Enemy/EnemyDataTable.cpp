#include <Windows.h>
#include <set>
#include "../../../../Manager/CsvLoader.h"
#include "EnemyDataTable.h"

namespace
{
	// CSVの置き場所（実行時のカレントディレクトリ基準。既存の PATH_MODEL などに合わせて調整）
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

void EnemyDataTable::Load(void)
{

	params_.clear();
	anims_.clear();
	spawns_.clear();
	attackRates_.clear();
	magics_.clear();

	LoadParam();
	LoadAnim();
	LoadSpawn();
	LoadAttack();
	LoadMagic();

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

EnemyMagicParam EnemyDataTable::GetMagicParam(const std::string& type)
{

	EnsureLoaded();

	auto it = magics_.find(type);
	if (it == magics_.end())
	{
		LogWarning("[EnemyDataTable] EnemyMagic.csv に " + type + " がありません。初期値を使用します");
		return EnemyMagicParam{};
	}

	return it->second;

}

float EnemyDataTable::GetAttackRate(const std::string& type, const std::string& motion)
{

	EnsureLoaded();

	// 行が無いモーションは等倍（警告は出さない）
	auto typeIt = attackRates_.find(type);
	if (typeIt == attackRates_.end())
	{
		return 1.0f;
	}

	auto motionIt = typeIt->second.find(motion);
	if (motionIt == typeIt->second.end())
	{
		return 1.0f;
	}

	return motionIt->second;

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

	// type,x,y,z,condition(conditionは省略可能)
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

		// 出現条件（5列目。省略可）
		if (r.size() >= 5)
		{
			s.condition = r[4];
		}

		spawns_.push_back(s);
	}

}

void EnemyDataTable::LoadAttack(void)
{

	// type,motion,rate
	auto rows = CsvLoader::Load(PATH_CSV + "EnemyAttack.csv");
	if (rows.empty())
	{
		LogWarning("[EnemyDataTable] EnemyAttack.csv を読み込めませんでした。ダメージ倍率は全て等倍になります");
	}

	for (const auto& r : rows)
	{
		if (r.size() < 3)
		{
			continue;
		}

		attackRates_[r[0]][r[1]] = CsvLoader::ToFloat(r[2], 1.0f);
	}

}

void EnemyDataTable::LoadMagic(void)
{

	// type,damage,speed,range,radius,castHeight,castOffset,castInterval,retreatDistance
	const size_t COLUMN_NUM = 9;

	auto rows = CsvLoader::Load(PATH_CSV + "EnemyMagic.csv");
	if (rows.empty())
	{
		LogWarning("[EnemyDataTable] EnemyMagic.csv を読み込めませんでした。魔法は初期値になります");
	}

	for (const auto& r : rows)
	{
		if (r.size() < COLUMN_NUM)
		{
			LogWarning("[EnemyDataTable] EnemyMagic.csv の列数が足りない行があります: " + r[0]);
			continue;
		}

		EnemyMagicParam p;
		p.damage = CsvLoader::ToInt(r[1], p.damage);
		p.speed = CsvLoader::ToFloat(r[2], p.speed);
		p.range = CsvLoader::ToFloat(r[3], p.range);
		p.radius = CsvLoader::ToFloat(r[4], p.radius);
		p.castHeight = CsvLoader::ToFloat(r[5], p.castHeight);
		p.castOffset = CsvLoader::ToFloat(r[6], p.castOffset);
		p.castInterval = CsvLoader::ToFloat(r[7], p.castInterval);
		p.retreatDistance = CsvLoader::ToFloat(r[8], p.retreatDistance);

		magics_[r[0]] = p;
	}

}