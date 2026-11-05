#include <Windows.h>
#include "../../../Manager/CsvLoader.h"
#include "PlayerDataTable.h"

namespace
{
	// CSVの置き場所（実行時のカレントディレクトリ基準。既存の PATH_MODEL などに合わせて調整）
	const std::string PATH_CSV = "Data/CSV/";

	// アニメCSVで共通扱いにするキー
	const std::string KEY_COMMON = "COMMON";

	const float DEG_TO_RAD = DX_PI_F / 180.0f;

	void LogWarning(const std::string& msg)
	{
		OutputDebugStringA((msg + "\n").c_str());
	}
}

void PlayerDataTable::Load(void)
{
	param_ = PlayerParam{};
	styles_.clear();
	weapons_.clear();
	anims_.clear();
	attackRates_.clear();

	LoadParam();
	LoadStyle();
	LoadWeapon();
	LoadAnim();
	LoadAttack();

	isLoaded_ = true;
}

PlayerParam PlayerDataTable::GetParam(void)
{
	EnsureLoaded();
	return param_;
}

PlayerStyle PlayerDataTable::GetStyle(const std::string& weapon)
{
	EnsureLoaded();

	auto it = styles_.find(weapon);
	if (it == styles_.end())
	{
		LogWarning("[PlayerDataTable] PlayerStyle.csv に " + weapon + " がありません。初期値を使用します");
		return PlayerStyle{};
	}

	return it->second;
}

WeaponParam PlayerDataTable::GetWeaponParam(const std::string& weapon)
{
	EnsureLoaded();

	auto it = weapons_.find(weapon);
	if (it == weapons_.end())
	{
		LogWarning("[PlayerDataTable] WeaponParam.csv に " + weapon + " がありません。初期値を使用します");
		return WeaponParam{};
	}

	return it->second;
}

float PlayerDataTable::GetAttackRate(const std::string& weapon, const std::string& motion)
{

	EnsureLoaded();

	// 行が無いモーションは等倍（警告は出さない）
	auto weaponIt = attackRates_.find(weapon);
	if (weaponIt == attackRates_.end())
	{
		return 1.0f;
	}

	auto motionIt = weaponIt->second.find(motion);
	if (motionIt == weaponIt->second.end())
	{
		return 1.0f;
	}

	return motionIt->second;

}

std::vector<PlayerAnimData> PlayerDataTable::GetAnims(const std::string& weapon)
{
	EnsureLoaded();

	std::vector<PlayerAnimData> result;

	// 共通アニメを先に入れる
	auto common = anims_.find(KEY_COMMON);
	if (common != anims_.end())
	{
		result = common->second;
	}

	// 武器専用アニメで上書き（同名があれば差し替え、無ければ追加）
	auto own = anims_.find(weapon);
	if (own == anims_.end())
	{
		LogWarning("[PlayerDataTable] PlayerAnim.csv に " + weapon + " がありません。共通アニメのみ使用します");
		return result;
	}

	for (const auto& anim : own->second)
	{
		bool isReplaced = false;
		for (auto& r : result)
		{
			if (r.name == anim.name)
			{
				r = anim;
				isReplaced = true;
				break;
			}
		}
		if (!isReplaced)
		{
			result.push_back(anim);
		}
	}

	return result;
}

void PlayerDataTable::EnsureLoaded(void)
{
	if (!isLoaded_)
	{
		Load();
	}
}

void PlayerDataTable::LoadParam(void)
{

	// type,hp,scale,startX,startY,startZ,localX,localY,localZ,knockbackPower,knockbackTime,
	// lineTopY,capsuleTopY,capsuleDownY,capsuleRadius,mainWeapon,subWeapon
	const size_t COLUMN_NUM = 17;

	auto rows = CsvLoader::Load(PATH_CSV + "PlayerParam.csv");
	if (rows.empty())
	{
		LogWarning("[PlayerDataTable] PlayerParam.csv を読み込めませんでした");
		return;
	}

	// 1行目だけ使用
	const auto& r = rows[0];
	if (r.size() < COLUMN_NUM)
	{
		LogWarning("[PlayerDataTable] PlayerParam.csv の列数が足りません");
		return;
	}

	PlayerParam p;
	p.hp = CsvLoader::ToInt(r[1], p.hp);
	p.scale = CsvLoader::ToFloat(r[2], p.scale);
	p.startPos = VGet(
		CsvLoader::ToFloat(r[3], p.startPos.x),
		CsvLoader::ToFloat(r[4], p.startPos.y),
		CsvLoader::ToFloat(r[5], p.startPos.z));
	p.localPos = VGet(
		CsvLoader::ToFloat(r[6], p.localPos.x),
		CsvLoader::ToFloat(r[7], p.localPos.y),
		CsvLoader::ToFloat(r[8], p.localPos.z));
	p.knockbackPower = CsvLoader::ToFloat(r[9], p.knockbackPower);
	p.knockbackTime = CsvLoader::ToFloat(r[10], p.knockbackTime);
	p.lineTopY = CsvLoader::ToFloat(r[11], p.lineTopY);
	p.capsuleTopY = CsvLoader::ToFloat(r[12], p.capsuleTopY);
	p.capsuleDownY = CsvLoader::ToFloat(r[13], p.capsuleDownY);
	p.capsuleRadius = CsvLoader::ToFloat(r[14], p.capsuleRadius);
	p.mainWeapon = r[15];
	p.subWeapon = r[16];

	param_ = p;

}

void PlayerDataTable::LoadStyle(void)
{
	// weapon,walkSpeed,dashSpeed,attackMove,parryAttackMove,parryedBackSpeed,comboMax,comboGraceTime
	// comboGraceTime は省略可能
	const size_t COLUMN_NUM = 7;

	auto rows = CsvLoader::Load(PATH_CSV + "PlayerStyle.csv");
	if (rows.empty())
	{
		LogWarning("[PlayerDataTable] PlayerStyle.csv を読み込めませんでした");
	}

	for (const auto& r : rows)
	{
		if (r.size() < COLUMN_NUM)
		{
			continue;
		}

		PlayerStyle s;
		s.walkSpeed = CsvLoader::ToFloat(r[1], s.walkSpeed);
		s.dashSpeed = CsvLoader::ToFloat(r[2], s.dashSpeed);
		s.attackMove = CsvLoader::ToFloat(r[3], s.attackMove);
		s.parryAttackMove = CsvLoader::ToFloat(r[4], s.parryAttackMove);
		s.parryedBackSpeed = CsvLoader::ToFloat(r[5], s.parryedBackSpeed);
		s.comboMax = CsvLoader::ToInt(r[6], s.comboMax);
		if (r.size() >= 8)
		{
			s.comboGraceTime = CsvLoader::ToFloat(r[7], s.comboGraceTime);
		}

		styles_[r[0]] = s;
	}
}

void PlayerDataTable::LoadWeapon(void)
{
	// weapon,slot,attachFrame,scale,posX,posY,posZ,rotXDeg,rotYDeg,rotZDeg,
	// colTopY,colDownY,colRadius,damage
	const size_t COLUMN_NUM = 14;

	auto rows = CsvLoader::Load(PATH_CSV + "WeaponParam.csv");
	if (rows.empty())
	{
		LogWarning("[PlayerDataTable] WeaponParam.csv を読み込めませんでした");
	}

	for (const auto& r : rows)
	{
		if (r.size() < COLUMN_NUM)
		{
			continue;
		}

		WeaponParam w;
		w.slot = r[1];
		w.attachFrame = CsvLoader::ToInt(r[2], w.attachFrame);
		w.scale = CsvLoader::ToFloat(r[3], w.scale);
		w.localPos = VGet(
			CsvLoader::ToFloat(r[4]),
			CsvLoader::ToFloat(r[5]),
			CsvLoader::ToFloat(r[6]));
		// CSVは度、内部はラジアン
		w.localRot = VGet(
			CsvLoader::ToFloat(r[7]) * DEG_TO_RAD,
			CsvLoader::ToFloat(r[8]) * DEG_TO_RAD,
			CsvLoader::ToFloat(r[9]) * DEG_TO_RAD);
		w.colTopY = CsvLoader::ToFloat(r[10], w.colTopY);
		w.colDownY = CsvLoader::ToFloat(r[11], w.colDownY);
		w.colRadius = CsvLoader::ToFloat(r[12], w.colRadius);
		w.damage = CsvLoader::ToInt(r[13], w.damage);

		weapons_[r[0]] = w;
	}
}

void PlayerDataTable::LoadAnim(void)
{
	// weapon,name,speed,path  （weapon は COMMON / SWORD / GREAT_SWORD ...）
	auto rows = CsvLoader::Load(PATH_CSV + "PlayerAnim.csv");
	if (rows.empty())
	{
		LogWarning("[PlayerDataTable] PlayerAnim.csv を読み込めませんでした");
	}

	for (const auto& r : rows)
	{
		if (r.size() < 4)
		{
			continue;
		}

		PlayerAnimData a;
		a.name = r[1];
		a.speed = CsvLoader::ToFloat(r[2], 30.0f);
		a.path = r[3];

		anims_[r[0]].push_back(a);
	}
}

void PlayerDataTable::LoadAttack(void)
{

	// weapon,motion,rate
	auto rows = CsvLoader::Load(PATH_CSV + "PlayerAttack.csv");
	if (rows.empty())
	{
		LogWarning("[PlayerDataTable] PlayerAttack.csv を読み込めませんでした。ダメージ倍率は全て等倍になります");
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