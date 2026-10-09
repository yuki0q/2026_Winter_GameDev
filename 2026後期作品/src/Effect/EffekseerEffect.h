#pragma once
#include <DxLib.h>
#include <EffekseerForDXLib.h>
#include <map>

class TopBase;

class EffekseerEffect
{
public:
    // シングルトン（生成・取得・削除）
    static void CreateInstance(void) { if (instance_ == nullptr) { instance_ = new EffekseerEffect(); } };
    static EffekseerEffect* GetInstance(void) { return instance_; };
    static void DeleteInstance(void) { if (instance_ != nullptr) { delete instance_; instance_ = nullptr; } }

private:
    // 静的インスタンス
    static EffekseerEffect* instance_;
private:
    // デフォルトコンストラクタをprivateにして、外部から生成できない様にする
    EffekseerEffect(void);
    // デストラクタも同様
    ~EffekseerEffect(void);

    // コピー・ムーブ操作を禁止
    EffekseerEffect(const EffekseerEffect&) = delete;
    EffekseerEffect& operator=(const EffekseerEffect&) = delete;
    EffekseerEffect(EffekseerEffect&&) = delete;
    EffekseerEffect& operator=(EffekseerEffect&&) = delete;

public:


	// 初期化
	void Init(void);
    
	// 更新
	void Update(void);

	// 描画
	void Draw(void);

	// 解放
	void Release(void);

    // デリート
	void Delete(void);

    void FireLightEffect(VECTOR& pos);
    void TrailIdleStart(TopBase* top, const int type);
    void TrailDashStart(TopBase* top, const int type);
    void TrailUpdate(TopBase* top, VECTOR& pos);
    void TrailStop(TopBase* top);

    void SkillStart(TopBase* top, const int type);
    void SkillUpdate(TopBase* top, VECTOR& pos);
    void SkillStop(TopBase* top);

    void ClearAllTrails(void);

private:

    // 火花エフェクト
    int effFireLight_;

    // コマの軌跡エフェクトハンドル
    int effTrailIdleBlue_;
    int effTrailDashBlue_;
    int effTrailIdleGreen_;
    int effTrailDashGreen_;
    int effTrailIdleYellow_;
    int effTrailDashYellow_;
    int effTrailIdleRed_;
    int effTrailDashRed_;

    // スキルエフェクト
    int effAttack_;
    int effShield_;
    int effStamina_;
    int effBalance_;

    int playFireLight_;

    int playNowTrail_;

    int playSkill_;

    std::map<TopBase*, int> activeTrails_;
    std::map<TopBase*, int> activeSkills_;
};
