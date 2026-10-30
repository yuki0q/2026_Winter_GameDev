#pragma once
#include "ItemBase.h"

// 敵を全滅させると出現するアイテム。取得するとクリアシーンへ遷移する
class ItemGold : public ItemBase
{
public:

    ItemGold(const VECTOR& pos, const VECTOR& scl, const VECTOR& rot);
    ~ItemGold(void) override;

    void Init(void) override;
    void Update(void) override;
    void Draw(void) override;

    // 取得時の効果
    void OnApplyEffect(std::shared_ptr<Player> player) override;

private:

    //// 回転速度（ラジアン/フレーム）
    //static constexpr float SPIN_SPEED_RAD = 0.03f;

    //// モデルが読み込めなかった時に表示する球の半径（スケール1.0のとき）
    //static constexpr float FALLBACK_RADIUS = 30.0f;
};