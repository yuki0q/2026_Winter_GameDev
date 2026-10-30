#pragma once
#include "SceneBase.h"
#include "../Object/Common/Transform.h"
#include "../Object/Actor/Charactor/Enemy/EnemyBase.h"

class Stage;
class Player;
class EnemyManager;
class UIGame;
class ItemManager;

class GameScene : public SceneBase
{
public:

	// コンストラクタ
	GameScene(void);

	// デストラクタ
	~GameScene(void) override;

	// 初期化
	void Init(void) override;

	// 更新
	void Update(void) override;

	// 描画
	void Draw(void) override;

	// 解放
	void Release(void) override;

private:

	void Collision(void);
	void CollisionWeaponToEnemy(void);
	void CollisionEnemyToPlayer(void);
	void CollisionPlayerAndEnemy(void);
	void CollisionEnemyAndEnemy(void);
	void CollisionParry(void);

	// プレイヤーから最も近い生きている敵を探す関数
	std::shared_ptr<EnemyBase> GetNearestEnemy(float maxDistance);

	std::unique_ptr <Stage> stage_;
	std::shared_ptr <Player> player_;
	std::shared_ptr <EnemyManager> enemyManager_;
	std::unique_ptr<UIGame> gameUI_;
	std::shared_ptr<ItemManager> itemManager_;

	// 1回の攻撃中にすでにヒットした敵のポインタ（アドレス）を記録するセット
	std::set<const void*> prevHit_;

	// 攻撃判定の前回状態を保持しておくフラグ
	bool prevAttackHit_;

	int shadowMapHandle_;

	int image3;
	int image2;
	int image1;
	int imgWin_;
	int imgBack_;

	// ヒットストップ用の残りフレーム数
	int hitStopTimer_;

	// ゲーム開始時に敵が1体以上いたか（敵が0体の状態で即クリアにならないようにする）
	bool hasEnemy_;

	// 全滅後の ItemGold を出現させたか（1回だけ出すためのフラグ）
	bool isGoldSpawned_;
};
