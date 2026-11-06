#include <DxLib.h>
#include <EffekseerForDXLib.h>
#include "../Manager/SceneManager.h"
#include "../Manager/InputManager.h"
#include "../Manager/Camera.h"
#include "../Manager/ResourceManager.h"
#include "../Manager/SoundManager.h"
#include "../Manager/Resource.h"
#include "../Object/Actor/Stage.h"
#include "../Object/Collider/ColliderBase.h"
#include "../Object/Collider/ColliderCapsule.h"
#include "../Object/Actor/Charactor/Player.h"
#include "../Object/Actor/Charactor/Enemy/EnemyManager.h"
#include "../Object/Actor/Charactor/Enemy/EnemyAlter.h"
#include "../Object/Actor/Weapon/WeaponManager.h"
#include "../Object/Actor/Item/ItemManager.h"
#include "../Object/Actor/Item/ItemHpPotion.h"
#include "../Object/Actor/UI/UIGame.h"
#include "../Object/Actor/Projectile/ProjectileManager.h"
#include "../Application.h"
#include "../Effect/EffekseerEffect.h"
#include "GameScene.h"

namespace
{
	// 敵の攻撃範囲
	const float NEAREST_ENEMY_RANGE = 200.0f;

	const int   HITSTOP_FRAME = 10;
	const float SHAKE_POWER_MAX = 0.008f;	// 画面幅の約0.8%
}

GameScene::GameScene(void)
	:
	shadowMapHandle_(0),
	SceneBase(),
	image3(0),
	image2(0),
	image1(0),
	imgBack_(0),
	imgWin_(0),
	hitStopTimer_(0),
	prevAttackHit_(false),
	hasEnemy_(false),
	isGoldSpawned_(false)
{
}

GameScene::~GameScene(void)
{
	player_ = nullptr;
	stage_ = nullptr;
	enemyManager_ = nullptr;
	itemManager_ = nullptr;
}

void GameScene::Init(void)
{
	EffekseerEffect::GetInstance()->CreateInstance();
	EffekseerEffect::GetInstance()->Init();
	
	player_ = std::make_shared<Player>();
	player_->Init();

	// 斬撃（飛び道具）
	projectileManager_ = std::make_shared<ProjectileManager>();
	projectileManager_->Init();
	player_->SetOnFireSlash([this](const VECTOR& pos, const VECTOR& dir)
		{
			projectileManager_->SpawnSlash(pos, dir);
		});

	enemyManager_ = std::make_shared<EnemyManager>();
	enemyManager_->Init();

	enemyManager_->SpawnFromCsv(player_);

	// EnemyAlter を倒すたびに回復薬をドロップ
	enemyManager_->SetOnEnemyDead(
		[this](EnemyBase::ENEMY_TYPE type, const VECTOR& pos)
		{
			if (!itemManager_) return;
			itemManager_->DropFromEnemy(EnemyBase::TypeToName(type), pos);
		});

	hasEnemy_ = !enemyManager_->GetEnemies().empty();
	isGoldSpawned_ = false;

	itemManager_ = std::make_shared<ItemManager>();
	itemManager_->Init();

	itemManager_->SpawnFromCsv("START");

	stage_ = std::make_unique<Stage>();
	stage_->Init();

	const ColliderBase* stageCollider =
		stage_->GetOwnCollider(static_cast<int>(Stage::COLLIDER_TYPE::MODEL));

	// ステージモデルのコライダーをプレイヤーに登録
	player_->AddHitCollider(stageCollider);
	enemyManager_->AddHitCollider(stageCollider);

	// エネミーのコライダーをプレイヤーに登録
	for (auto& enemy : enemyManager_->GetEnemies())
	{
		player_->
			AddHitCollider(enemy->GetOwnCollider(static_cast<int>
				(CharactorBase::COLLIDER_TYPE::CAPSULE)));
	}

	// プレイヤーのコライダーをエネミーに登録
	enemyManager_->
		AddHitCollider(player_->GetOwnCollider(static_cast<int>
			(CharactorBase::COLLIDER_TYPE::CAPSULE)));

	// 追従カメラモードに変更
	Camera* camera = sceMng_.GetCamera();
	camera->SetFollow(&player_->GetTransform());
	camera->ChangeMode(Camera::MODE::FOLLOW);
	camera->AddHitCollider(stageCollider);

	// シャドウマップハンドルの作成
	shadowMapHandle_ = MakeShadowMap(2048, 2048);

	// シャドウマップが想定するライトの方向もセット
	SetShadowMapLightDirection(shadowMapHandle_, VGet(0.5f, -0.5f, 0.5f));

	// シャドウマップに描画する範囲を設定
	SetShadowMapDrawArea(shadowMapHandle_, VGet(-5000.0f, -70.0f, -6000.0f), 
		VGet(6000.0f, 1500.0f, 8000.0f));

	// タイトル画面に必要なサウンドをロード
	SoundManager::GetInstance()->LoadSceneSound(LoadScene::GAME);

	// タイトルBGMを再生（自動でループ再生されます）
	SoundManager::GetInstance()->PlayBGM(SoundID::BGM_BATTLE);
	SoundManager::GetInstance()->SetBgmVolume(160); 

	gameUI_ = std::make_unique<UIGame>(player_, enemyManager_);
	gameUI_->Init();
}

void GameScene::Update(void)
{
	stage_->Update();

	// バッグ操作。開いている間はゲーム内の更新を止める
	if (itemManager_)
	{
		const bool wasBagOpen = itemManager_->IsBagOpen();
		itemManager_->UpdateBag(player_);

		// 開いている間、および開閉したフレームは、以降の更新（ポーズ判定を含む）を行わない
		if (wasBagOpen || itemManager_->IsBagOpen())
		{
			return;
		}
	}

	if (hitStopTimer_ > 0)
	{
		// ヒットストップ中はタイマーを減らすのみで、
		// プレイヤーや敵の Update、および Collision は実行しない
		hitStopTimer_--;

		// 残りフレームに比例して減衰させる(0になれば揺れも止まる)
		float rate = static_cast<float>(hitStopTimer_) / HITSTOP_FRAME;
		//sceMng_.SetShakePower(SHAKE_POWER_MAX * rate);
	}
	else
	{
		//sceMng_.SetShakePower(0.0f);

		bool isEnemyPreAttacking = false;

		// 一定範囲（例: 300.0f 以内）に攻撃前の敵がいるか走査
		for (auto& enemy : enemyManager_->GetEnemies())
		{
			if (!enemy || enemy->IsDead()) continue;

			if (enemy->GetCurrentState() != EnemyBase::STATE::PREV_ATTACK)continue;
			
			// プレイヤーとの距離が近ければ警告対象とする
			VECTOR diff = VSub(enemy->GetTransform().pos, player_->GetTransform().pos);
			if (VDot(diff, diff) < (NEAREST_ENEMY_RANGE * NEAREST_ENEMY_RANGE))
			{
				isEnemyPreAttacking = true;
				break;
			}
		}

		// プレイヤーに警告状態を伝える
		player_->SetWarning(isEnemyPreAttacking);

		// 通常時の更新処理
		player_->Update();
		enemyManager_->Update();
		itemManager_->Update();
		projectileManager_->Update();

		Collision();
	}

	if (gameUI_)
	{
		gameUI_->Update();
	}

	if (player_ && player_->GetDead())
	{
		sceMng_.ChangeScene(SceneManager::SCENE_ID::GAMEOVER);
		return;
	}

	// すべての敵を倒したら ItemGold を出現させる（1回だけ）
	if (hasEnemy_ && !isGoldSpawned_ &&
		enemyManager_ && enemyManager_->IsAllEnemyDead())
	{
		itemManager_->SpawnFromCsv("CLEAR");
		isGoldSpawned_ = true;
	}

	// ItemGold を取得したらクリアシーンへ切り替え
	if (itemManager_ && itemManager_->IsGoldCollected())
	{
		sceMng_.ChangeScene(SceneManager::SCENE_ID::CLEAR);
		return;
	}

	// シーン遷移
	auto& ins = InputManager::GetInstance();

	bool IsPause = ins.IsActionTrgDown(InputManager::ACTION::PAUSE);
	if (!IsPause)
	{
		return;
	}
	
	SoundManager::GetInstance()->PlaySE(SoundID::SE_WINDOW);
	sceMng_.ChangeScene(SceneManager::SCENE_ID::PAUSE);
}

void GameScene::Draw(void)
{
	// デバッグ用
	DrawFormatString(10, 10, 0xffffff, "ゲームシーン");

	// シャドウマップへの描画の準備
	ShadowMap_DrawSetup(shadowMapHandle_);

	stage_->Draw();
	player_->Draw();
	enemyManager_->Draw();
	projectileManager_->Draw();
	itemManager_->Draw();

	// シャドウマップへの描画を終了
	ShadowMap_DrawEnd();
	
	// 描画に使用するシャドウマップを設定
	SetUseShadowMap(0, shadowMapHandle_);

	stage_->Draw();
	player_->Draw();
	enemyManager_->Draw();
	projectileManager_->Draw();
	itemManager_->Draw();

	// 描画に使用するシャドウマップの設定を解除
	SetUseShadowMap(0, -1);

	// 3Dの奥行き判定を一時的に無効化する
	SetUseZBuffer3D(FALSE);
	SetWriteZBuffer3D(FALSE);

	// 終わったら3D用に設定を戻す
	SetUseZBuffer3D(TRUE);
	SetWriteZBuffer3D(TRUE);

	if (itemManager_)
	{
		itemManager_->DrawPickupPrompt();
	}

	// 3D描画が終わったあとの2D描画フェーズでUIを描画する
	if (gameUI_)
	{
		gameUI_->Draw();
	}

	// バッグ画面（UIの上に重ねる）
	if (itemManager_)
	{
		itemManager_->DrawBag();
	}
}

void GameScene::Release(void)
{
	if (gameUI_)
	{
		gameUI_->Release();
		gameUI_.reset();
	}

	DeleteShadowMap(shadowMapHandle_);

	stage_->Release();
	player_->Release();
	enemyManager_->Release();
	itemManager_->Release();

	if (projectileManager_) { projectileManager_->Release(); }

	SoundManager::GetInstance()->StopBGM();
	SoundManager::GetInstance()->DeleteSceneSound(LoadScene::GAME);

	EffekseerEffect::GetInstance()->DeleteInstance();
	//sceMng_.SetShakePower(0.0f);
}

void GameScene::Collision(void)
{
	CollisionPlayerAndEnemy();
	CollisionEnemyAndEnemy();
	CollisionWeaponToEnemy();
	CollisionEnemyToPlayer();
	CollisionParry();
	projectileManager_->CollisionToEnemies(enemyManager_->GetEnemies());

	if (itemManager_)
	{
		itemManager_->CheckCollection(player_);
	}
}

void GameScene::CollisionWeaponToEnemy(void)
{
	auto rightWeapon = player_->weaponManager_->GetWeapon(WEAPON_SLOT::RIGHT_HAND);

	if (!rightWeapon || !rightWeapon->IsAttackEnable())
	{
		prevHit_.clear();
		return;
	}

	bool currentAttackEnable = rightWeapon->IsAttackEnable();

	if (prevHit_.empty() && currentAttackEnable)
	{
		// 半径以内にいる最も近い敵を探す
		auto nearestEnemy = GetNearestEnemy(NEAREST_ENEMY_RANGE);
		if (nearestEnemy)
		{
			player_->LookAt(nearestEnemy->GetTransform().pos);
		}
	}

	const ColliderCapsule* weaponCapsule =
		dynamic_cast<const ColliderCapsule*>(rightWeapon->GetCollider());

	if (!weaponCapsule)
	{
		return;
	}

	// 今フレームで武器と接触している敵を一時的に記録する
	std::set<const void*> currentHitEnemies;

	for (auto& enemy : enemyManager_->GetEnemies())
	{
		if (!enemy || enemy->IsDead())continue;
		
			// 敵のポインタをキーとして、今回の攻撃ですでにヒットしているかチェック
			const void* enemyPtr = static_cast<const void*>(enemy.get());

			// 敵のカプセルコライダを取得
			const ColliderCapsule* enemyCapsule =
				dynamic_cast<const ColliderCapsule*>(
					enemy->GetOwnCollider(static_cast<int>(CharactorBase::COLLIDER_TYPE::CAPSULE)));

			// 武器コライダと敵コライダが衝突しているか判定
			if (!enemyCapsule || !weaponCapsule->IsHit(enemyCapsule)) continue;

			// 今フレームで当たっているリストに追加
			currentHitEnemies.insert(enemyPtr);

			// 「前フレームのリストに入っていなかった」場合のみダメージを与える
			if (prevHit_.find(enemyPtr) != prevHit_.end()) continue;
			enemy->OnDamage(player_->CalcAttackDamage(rightWeapon->GetDamage())
				, player_->GetTransform().pos);
		
	}
	// 次のフレームのために、今フレームの接触状態を「前フレームの履歴」として保存する
	prevHit_ = currentHitEnemies;
}

void GameScene::CollisionEnemyToPlayer(void)
{
	if (player_->IsParry()) return;

	bool isAnyEnemyHittingNow = false;

	for (auto& enemy : enemyManager_->GetEnemies())
	{
		if (!enemy || enemy->IsDead() || !enemy->IsAttackColliderEnable())continue;
		
			const ColliderBase* enemyAttackCol = enemy->GetAttackCollider();
			if (!enemyAttackCol) continue;

			const ColliderCapsule* enemyAttackCapsule = dynamic_cast<const ColliderCapsule*>(enemyAttackCol);
			if (!enemyAttackCapsule) continue;

			// プレイヤーのカプセルコライダを取得
			const ColliderCapsule* playerCapsule = dynamic_cast<const ColliderCapsule*>(
				player_->GetOwnCollider(static_cast<int>(CharactorBase::COLLIDER_TYPE::CAPSULE))
				);

			if (!enemyAttackCapsule || !playerCapsule || !enemyAttackCapsule->IsHit(playerCapsule)) continue;
				isAnyEnemyHittingNow = true;

			// 「前フレームのリストに入っていなかった」場合のみダメージを与える
			if (!isAnyEnemyHittingNow || prevAttackHit_) continue;
			player_->OnDamage(enemy->CalcAttackDamage(), enemy->GetTransform().pos);
		
	}
	// 状態を更新
	prevAttackHit_ = isAnyEnemyHittingNow;
}

void GameScene::CollisionPlayerAndEnemy(void)
{
	// プレイヤーのカプセルコライダを取得
	const ColliderCapsule* playerCapsule = dynamic_cast<const ColliderCapsule*>(
		player_->GetOwnCollider(static_cast<int>(CharactorBase::COLLIDER_TYPE::CAPSULE))
		);

	if (!playerCapsule) return;

	VECTOR playerPos = player_->GetTransform().pos;

	for (auto& enemy : enemyManager_->GetEnemies())
	{
		if (!enemy || enemy->IsDead()) continue;

		// 敵のカプセルコライダを取得
		const ColliderCapsule* enemyCapsule = dynamic_cast<const ColliderCapsule*>(
			enemy->GetOwnCollider(static_cast<int>(CharactorBase::COLLIDER_TYPE::CAPSULE))
			);

		if (!enemyCapsule) continue;

		// カプセル同士が衝突（接触）しているか判定
		if (!playerCapsule->IsHit(enemyCapsule))
		{
			continue;
		}

		// 簡易かつ安定したカプセル間のめり込み防止（XZ平面での中心間距離ベースの押し戻し）
		VECTOR playerCenter = playerCapsule->GetCenter();
		VECTOR enemyCenter = enemyCapsule->GetCenter();

		// 2者間の高低差を無視して水平方向（XZ）のベクトルを計算
		VECTOR diff = VSub(playerCenter, enemyCenter);
		diff.y = 0.0f;
		float dist = VSize(diff);

		// お互いの半径の合計
		float minDistance = playerCapsule->GetRadius() + enemyCapsule->GetRadius();

		// 実際にめり込んでいる距離
		float overlap = minDistance - dist;

		if (overlap > 0.0f && dist > 0.0001f)
		{
			// 押し戻し方向の正規化ベクトル
			VECTOR pushDir = VScale(diff, 1.0f / dist);

			// お互いに半分ずつ押し戻す（またはプレイヤー側・敵側の移動量に応じて配分）
			float halfOverlap = overlap * 0.5f;

			// プレイヤーの位置を押し戻す
			playerPos = VAdd(playerPos, VScale(pushDir, halfOverlap));
			player_->SetPosition(playerPos);

			// 敵の位置を反対方向に押し戻す
			VECTOR enemyPos = enemy->GetTransform().pos;
			enemyPos = VSub(enemyPos, VScale(pushDir, halfOverlap));
			enemy->SetPosition(enemyPos);
		}
	}
}

void GameScene::CollisionEnemyAndEnemy(void)
{
	auto& enemies = enemyManager_->GetEnemies();
	size_t size = enemies.size();

	// すべての敵の組み合わせをチェック
	for (size_t i = 0; i < size; ++i)
	{
		auto& enemyA = enemies[i];
		if (!enemyA || enemyA->IsDead()) continue;

		const ColliderCapsule* capsuleA = dynamic_cast<const ColliderCapsule*>(
			enemyA->GetOwnCollider(static_cast<int>(CharactorBase::COLLIDER_TYPE::CAPSULE))
			);
		if (!capsuleA) continue;

		for (size_t j = i + 1; j < size; ++j)
		{
			auto& enemyB = enemies[j];
			if (!enemyB || enemyB->IsDead()) continue;

			const ColliderCapsule* capsuleB = dynamic_cast<const ColliderCapsule*>(
				enemyB->GetOwnCollider(static_cast<int>(CharactorBase::COLLIDER_TYPE::CAPSULE))
				);
			if (!capsuleB) continue;

			// 敵同士のカプセルがヒットしているか判定
			if (!capsuleA->IsHit(capsuleB)) continue;

			// 座標は毎回 Transform から取得する
			// （同じフレームで前の組み合わせにより押し戻された結果を反映するため）
			VECTOR posA = enemyA->GetTransform().pos;
			VECTOR posB = enemyB->GetTransform().pos;

			// 高低差を無視して水平方向（XZ）の距離で判定
			VECTOR diff = VSub(posA, posB);
			diff.y = 0.0f;
			float dist = VSize(diff);

			// お互いの半径の合計と、実際にめり込んでいる距離
			float minDistance = capsuleA->GetRadius() + capsuleB->GetRadius();
			float overlap = minDistance - dist;
			if (overlap <= 0.0f) continue;

			// 押し戻し方向（B→A）。座標が完全に一致している場合は固定方向に逃がす
			VECTOR pushDir = { 1.0f, 0.0f, 0.0f };
			if (dist > 0.0001f)
			{
				pushDir = VScale(diff, 1.0f / dist);
			}

			float halfOverlap = overlap * 0.5f;

			// 敵Aは B から遠ざかる方向へ
			enemyA->SetPosition(VAdd(posA, VScale(pushDir, halfOverlap)));

			// 敵Bは反対方向（A から遠ざかる方向）へ
			enemyB->SetPosition(VSub(posB, VScale(pushDir, halfOverlap)));
		}
	}
}

void GameScene::CollisionParry(void)
{
	// プレイヤーがパリィ中でなければ何もしない
	if (!player_->IsParry()) return;

	VECTOR effectPos = player_->GetTransform().pos; // デフォルト（プレイヤーの位置）

	VECTOR effectRot = { 0.0f, 0.0f, 0.0f }; // 必要に応じた角度（ラジアン）

	// 半径以内にいる最も近い敵を探す
	auto nearestEnemy = GetNearestEnemy(NEAREST_ENEMY_RANGE);
	if (nearestEnemy)
	{
		player_->LookAt(nearestEnemy->GetTransform().pos);
	}

	for (auto& enemy : enemyManager_->GetEnemies())
	{
		// 敵の攻撃判定があるのかを判定
		if (!enemy || enemy->IsDead() || !enemy->IsAttackColliderEnable()) continue;

			const ColliderBase* enemyAttackCol = enemy->GetAttackCollider();
			if (!enemyAttackCol) continue;

			const ColliderCapsule* enemyAttackCapsule = dynamic_cast<const ColliderCapsule*>(enemyAttackCol);
			if (!enemyAttackCapsule) continue;

			// プレイヤーのカプセルコライダ（またはパリィ専用コライダ）を取得
			const ColliderCapsule* playerCapsule = dynamic_cast<const ColliderCapsule*>(
				player_->GetOwnCollider(static_cast<int>(CharactorBase::COLLIDER_TYPE::CAPSULE))
				);

			// 敵の攻撃コライダとプレイヤーが接触しているか判定
			if (!enemyAttackCapsule || !playerCapsule || !enemyAttackCapsule->IsHit(playerCapsule)) continue;

			// 敵の攻撃判定を即座にオフにする
			enemy->SetAttackColliderEnable(false);

			// 敵をひるませる
			enemy->OnDamage(0, player_->GetTransform().pos);

			player_->ClearParry();

			// 左手の武器（盾）のコライダから座標を取得する
			auto leftWeapon = player_->weaponManager_->GetWeapon(WEAPON_SLOT::LEFT_HAND);
			if (leftWeapon)
			{
				const ColliderBase* leftCol = leftWeapon->GetCollider();
				if (!leftCol) continue;

				const ColliderCapsule* leftCapsule = dynamic_cast<const ColliderCapsule*>(leftCol);
				if (leftCapsule)
				{
					// 左手武器（盾）のカプセルコライダの中心座標を取得
					effectPos = leftCapsule->GetCenter();
				}
			}

			// 敵の攻撃コライダ（または敵の中心）の座標を取得
			VECTOR enemyPos = enemyAttackCapsule->GetCenter();

			// 盾から敵へ向かう方向ベクトルを計算
			VECTOR dir = VSub(enemyPos, effectPos);

			// 方向ベクトルからY軸（水平）とX軸（上下）の回転角（ラジアン）を算出
			float rotY = atan2f(dir.x, dir.z);
			float distXZ = sqrtf(dir.x * dir.x + dir.z * dir.z);
			float rotX = -atan2f(dir.y, distXZ);

			VECTOR effectScl = { 7.0f, 7.0f, 7.0f };
			VECTOR effectRot = { rotX, rotY, 0.0f };

			// 取得した座標にエフェクトを発生させる
			EffekseerEffect::GetInstance()->FireLightEffect(effectPos, effectScl, effectRot);

			hitStopTimer_ = HITSTOP_FRAME;

			break; // 1回のフレームで1つの敵の攻撃をパリィしたらループを抜ける
	}
}

std::shared_ptr<EnemyBase> GameScene::GetNearestEnemy(float maxDistance)
{
	std::shared_ptr<EnemyBase> nearestEnemy = nullptr;
	float minDistSq = maxDistance * maxDistance; // 判定距離の二乗
	VECTOR playerPos = player_->GetTransform().pos;

	for (auto& enemy : enemyManager_->GetEnemies())
	{
		if (!enemy || enemy->IsDead()) continue;

		VECTOR diff = VSub(enemy->GetTransform().pos, playerPos);
		diff.y = 0.0f;
		float distSq = VDot(diff, diff);

		if (distSq < minDistSq)
		{
			minDistSq = distSq;
			nearestEnemy = enemy;
		}
	}
	return nearestEnemy;
}
