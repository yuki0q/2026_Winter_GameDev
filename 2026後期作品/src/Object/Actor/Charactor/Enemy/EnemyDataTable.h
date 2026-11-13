#pragma once
#include <DxLib.h>
#include <map>
#include <string>
#include <vector>

// 敵のパラメータ（EnemyParam.csv）
// 初期値はCSVが読めなかった場合のフォールバック値
struct EnemyParam
{
	int   hp = 50;
	float scale = 1.0f;
	float walkSpeed = 3.0f;
	float runSpeed = 7.5f;
	float searchRadius = 1000.0f;
	float searchViewAngle = 45.0f;
	float attackRadius = 100.0f;
	float attackColRadius = 15.0f;
	int   attackStartFrame = -1;
	int   attackEndFrame = -1;
	float knockbackPower = 10.0f;
	float knockbackTime = 0.2f;
	float lineTopY = 120.0f;
	float capsuleTopY = 180.0f;
	float capsuleDownY = 40.0f;
	float capsuleRadius = 35.0f;
};

// アニメーション（EnemyAnim.csv）
struct EnemyAnimData
{
	std::string name;	// 「IDLE」などの名前（enumとの対応はキャラクター側で行う）
	float speed = 30.0f;
	std::string path;	// PATH_MODEL からの相対パス
};

// スポーン（EnemySpawn.csv）
struct EnemySpawnData
{
	std::string type;	// 「ALTER」など
	VECTOR pos = { 0.0f, 0.0f, 0.0f };

	// 出現条件（省略または START: ゲーム開始時 / AFTER_ALTER: ALTER をすべて倒した後 など）
	std::string condition;
};

// 魔法（飛び道具）を撃つ敵のパラメータ（EnemyMagic.csv）
struct EnemyMagicParam
{
	int   damage = 10;				// 基本ダメージ（攻撃モーションごとの倍率は EnemyAttack.csv）
	float speed = 900.0f;			// 飛ぶ速さ（1秒あたりの距離）
	float range = 2000.0f;			// 飛距離（ここまで飛ぶと消える）
	float radius = 30.0f;			// 当たり判定の半径
	float castHeight = 120.0f;		// 発射位置の高さ（敵の足元から）
	float castOffset = 60.0f;		// 発射位置の前方へのずれ
	float castInterval = 3.0f;		// 次に撃てるまでの時間（秒）
	float retreatDistance = 350.0f;	// プレイヤーがこの距離より近づくと、距離を取る
};

// 敵関連のCSVデータをまとめて管理する
class EnemyDataTable
{

public:

	// 全CSVを読み込み直す（デバッグ中の再読み込みにも使用）
	static void Load(void);

	// 敵種別ごとのパラメータ（無ければ初期値）
	static EnemyParam GetParam(const std::string& type);

	// 敵種別ごとのアニメーション一覧（無ければ空）
	static const std::vector<EnemyAnimData>& GetAnims(const std::string& type);

	// スポーン情報一覧
	static const std::vector<EnemySpawnData>& GetSpawns(void);

	// 敵種別ごとの魔法パラメータ（EnemyMagic.csv。無ければ初期値）
	static EnemyMagicParam GetMagicParam(const std::string& type);

	// 敵種別・攻撃モーションごとのダメージ倍率（EnemyAttack.csv）
	// motion は「ATTACK_1」「ATTACK_2」など。行が無ければ 1.0
	static float GetAttackRate(const std::string& type, const std::string& motion);

private:

	static void EnsureLoaded(void);
	static void LoadParam(void);
	static void LoadAnim(void);
	static void LoadSpawn(void);
	static void LoadAttack(void);
	static void LoadMagic(void);

	static inline bool isLoaded_ = false;
	static inline std::map<std::string, EnemyParam> params_;
	static inline std::map<std::string, std::vector<EnemyAnimData>> anims_;
	static inline std::vector<EnemySpawnData> spawns_;
	static inline std::map<std::string, std::map<std::string, float>> attackRates_;
	static inline std::map<std::string, EnemyMagicParam> magics_;
};