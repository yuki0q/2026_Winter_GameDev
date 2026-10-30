#pragma once
#include <DxLib.h>
#include <map>
#include <string>
#include <vector>

// パラメータ（PlayerParam.csv）
// 初期値はCSVが読めなかった場合のフォールバック値
struct PlayerParam
{
	int   hp = 100;
	float scale = 1.0f;
	VECTOR startPos = { 0.0f, 300.0f, 0.0f };
	VECTOR localPos = { 100.0f, 0.0f, 100.0f };
	float knockbackPower = 10.0f;
	float knockbackTime = 0.2f;
	float lineTopY = 120.0f;
	float capsuleTopY = 160.0f;
	float capsuleDownY = 25.0f;
	float capsuleRadius = 25.0f;
	std::string mainWeapon = "SWORD";	// 右手に持つ武器（アニメ・動きの種類を決める）
	std::string subWeapon = "SHIELD";	// 左手に持つ武器（無ければ NONE）
};

// メイン武器の種類ごとに変わる動きのパラメータ（PlayerStyle.csv）
struct PlayerStyle
{
	float walkSpeed = 5.0f;
	float dashSpeed = 15.0f;
	float attackMove = 4.0f;		// 攻撃中の前進量
	float parryAttackMove = 4.0f;	// パリィ攻撃中の前進量
	float parryedBackSpeed = 20.0f;	// パリィ後の後退初速
	int   comboMax = 4;				// コンボ段数
	float comboGraceTime = 3.0f;	// 攻撃が途切れてからコンボが続けられる猶予時間（秒）
};

// 武器1本ごとのパラメータ（WeaponParam.csv）
struct WeaponParam
{
	std::string slot = "RIGHT_HAND";	// RIGHT_HAND / LEFT_HAND / BACK
	int   attachFrame = -1;				// 取り付けるキャラモデルのフレーム番号
	float scale = 1.0f;
	VECTOR localPos = { 0.0f, 0.0f, 0.0f };
	VECTOR localRot = { 0.0f, 0.0f, 0.0f };	// ラジアン（CSVでは度で指定）
	float colTopY = 0.0f;				// 攻撃判定カプセルの上端（武器ローカルY）
	float colDownY = 0.0f;				// 攻撃判定カプセルの下端（武器ローカルY）
	float colRadius = 0.0f;
	int   damage = 10;
};


// アニメーション1件分（PlayerAnim.csv）
struct PlayerAnimData
{
	std::string name;	// 「IDLE」などの名前（enumとの対応はキャラクター側で行う）
	float speed = 30.0f;
	std::string path;	// PATH_MODEL からの相対パス
};

class PlayerDataTable
{
public:

	// 全CSVを読み込み直す（デバッグ中の再読み込みにも使用）
	static void Load(void);

	// プレイヤー共通パラメータ
	static PlayerParam GetParam(void);

	// 武器種別ごとの動きのパラメータ（weapon は "SWORD" など）
	static PlayerStyle GetStyle(const std::string& weapon);

	// 武器1本ごとのパラメータ
	static WeaponParam GetWeaponParam(const std::string& weapon);

	// 武器種別ごとのアニメーション一覧
	// COMMON（共通）に武器専用の行を重ね、同じ名前は武器専用が優先される
	static std::vector<PlayerAnimData> GetAnims(const std::string& weapon);

private:

	static void EnsureLoaded(void);
	static void LoadParam(void);
	static void LoadStyle(void);
	static void LoadWeapon(void);
	static void LoadAnim(void);

	static inline bool isLoaded_ = false;
	static inline PlayerParam param_;
	static inline std::map<std::string, PlayerStyle> styles_;
	static inline std::map<std::string, WeaponParam> weapons_;
	static inline std::map<std::string, std::vector<PlayerAnimData>> anims_;
};

