#include <DxLib.h>
#include <EffekseerForDXLib.h>
#include "../Utility/AsoUtility.h"
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "../Application.h"
#include "../Object/Common/Transform.h"
#include "../Object/Collider/ColliderModel.h"
#include "../Object/Collider/ColliderSphere.h"
#include "../Object/Collider/ColliderBase.h"
#include "Camera.h"

Camera::Camera(void)
	:
	followTransform_(nullptr),
	mode_(MODE::NONE),
	angles_(AsoUtility::VECTOR_ZERO),
	rotY_(Quaternion::Identity()),
	targetPos_(AsoUtility::VECTOR_ZERO),
	sceMng_(SceneManager::GetInstance()),
	followDist_(400.0),
	prePos_(),
	isCollision_()
	//rot_(Quaternion::Identity()),
	// //pos_(AsoUtility::VECTOR_ZERO),
	//cameraUp_(AsoUtility::DIR_U)
{
	// DxLibの初期設定では、
	// カメラの位置が x = 320.0f, y = 240.0f, z = (画面のサイズによって変化)、
	// 注視点の位置は x = 320.0f, y = 240.0f, z = 1.0f
	// カメラの上方向は x = 0.0f, y = 1.0f, z = 0.0f
	// 右上位置からZ軸のプラス方向を見るようなカメラ
}

Camera::~Camera(void)
{
}

void Camera::Update(void)
{
}

void Camera::SetBeforeDraw(void)
{

	// クリップ距離を設定する(SetDrawScreenでリセットされる)
	SetCameraNearFar(VIEW_NEAR, VIEW_FAR);

	// 更新前情報
	prePos_ = transform_.pos;

	switch (mode_)
	{
	case Camera::MODE::FIXED_POINT:
		SetBeforeDrawFixedPoint();
		break;
	case Camera::MODE::FREE:
		SetBeforeDrawFree();
		break;
	case Camera::MODE::FOLLOW:
		SetBeforeDrawFollow();
		break;
	}

	// カメラの設定(位置と注視点による制御)
	SetCameraPositionAndTargetAndUpVec(
		transform_.pos,
		targetPos_,
		transform_.quaRot.GetUp()
	);

	// DXライブラリのカメラとEffekseerのカメラを同期する。
	Effekseer_Sync3DSetting();

}

void Camera::DrawDebug(void)
{
}

void Camera::Release(void)
{
}

void Camera::SetFollow(const Transform* follow)
{
	followTransform_ = follow;
}

void Camera::TriggerZoomIn(const VECTOR& zoomPoint1, const VECTOR& zoomPoint2, float duration)
{
	isZooming_ = true;
	zoomTargetPoint_ = AsoUtility::Lerp(zoomPoint1, zoomPoint2, 0.15);
	zoomTimer_ = duration;
}

void Camera::InitLoad(void)
{
}

void Camera::InitTransform(void)
{
	// カメラの位置
	transform_.pos = AsoUtility::VECTOR_ZERO;

	// カメラの角度
	transform_.quaRot = Quaternion::Identity();

	// カメラの上方向
	transform_.quaRot.GetUp();
}

void Camera::InitCollider(void)
{
	// 主に地面との衝突で使用する球体コライダ
	ColliderSphere* colliderSphere = new ColliderSphere(
		ColliderBase::TAG::CAMERA,
		&transform_,
		AsoUtility::VECTOR_ZERO,
		COL_CAPSULE_SPHERE
	);
	ownColliders_.emplace(
		static_cast<int>(COLLIDER_TYPE::SPHERE), colliderSphere);
}

void Camera::InitAnimation(void)
{
}

void Camera::InitPost(void)
{
	ChangeMode(MODE::FIXED_POINT);

	isCollision_ = false;

	followDist_ = fabsf(FOLLOW_CAMERA_LOCAL_POS.z);
}

const VECTOR& Camera::GetPos(void) const
{
	return transform_.pos;
}

const VECTOR& Camera::GetAngles(void) const
{
	return angles_;
}

const VECTOR& Camera::GetTargetPos(void) const
{
	return targetPos_;
}

const Quaternion& Camera::GetQuaRot(void) const
{
	return transform_.quaRot;
}

const Quaternion& Camera::GetQuaRotY(void) const
{
	return rotY_;
}

VECTOR Camera::GetForward(void) const
{
	return VNorm(VSub(targetPos_, transform_.pos));
}

void Camera::ChangeMode(MODE mode)
{

	// カメラの初期設定
	SetDefault();

	// カメラモードの変更
	mode_ = mode;

	// 変更時の初期化処理
	switch (mode_)
	{
	case Camera::MODE::FIXED_POINT:
		break;
	case Camera::MODE::FREE:
		break;
	case Camera::MODE::FOLLOW:
		break;
	}

}

void Camera::ProcessZoom(void)
{
	auto& ins = InputManager::GetInstance();
	int wheel = ins.GetMouseWheel();

	if (wheel != 0)
	{
		followDist_ -= wheel * ZOOM_SPEED;

		// 距離の制限範囲内にクランプ
		if (followDist_ < CAMERA_DIST_MIN) { followDist_ = CAMERA_DIST_MIN; }
		if (followDist_ > CAMERA_DIST_MAX) { followDist_ = CAMERA_DIST_MAX; }
	}

	if (ins.IsNew(KEY_INPUT_RETURN))
	{
		SetDefault();
	}
}

void Camera::SetDefault(void)
{

	// カメラの初期設定
	transform_.pos = DERFAULT_POS;

	// カメラ角
	angles_ = DERFAULT_ANGLES;
	transform_.quaRot = Quaternion::Identity();

	// 注視点
	targetPos_ = AsoUtility::VECTOR_ZERO;

	// カメラの上方向
	transform_.quaRot.GetUp();

	followDist_ = DEFAULT_FOLLOW_DIST;
}

void Camera::SyncFollow(void)
{

	// 同期先の位置
	VECTOR pos = followTransform_->pos;

	// Y軸
	rotY_ = Quaternion::AngleAxis(angles_.y, AsoUtility::AXIS_Y);

	// Y軸 + X軸
	transform_.quaRot = rotY_.Mult(Quaternion::AngleAxis(angles_.x, AsoUtility::AXIS_X));

	VECTOR localPos;

	// 注視点
	localPos = transform_.quaRot.PosAxis(FOLLOW_TARGET_LOCAL_POS);
	targetPos_ = VAdd(pos, localPos);

	VECTOR currentFollowCameraLocalPos = FOLLOW_CAMERA_LOCAL_POS;
	currentFollowCameraLocalPos.z = -followDist_;

	// カメラ位置
	localPos = transform_.quaRot.PosAxis(currentFollowCameraLocalPos);
	transform_.pos = VAdd(pos, localPos);

	// カメラの上方向
	transform_.quaRot.GetUp();

}

void Camera::ProcessRot(bool isLimit)
{
	auto& ins = InputManager::GetInstance();
	if (GetJoypadNum() == 0)
	{
		
		// 左Altを押していない間はマウスが中央に固定
		if (!ins.IsNew(KEY_INPUT_LALT) && !(sceMng_.GetSceneID() == SceneManager::SCENE_ID::PAUSE))
		{
			SetMouseDispFlag(FALSE);
			RotMouse(isLimit);
			
		}
		else
		{
			SetMouseDispFlag(TRUE);
			// 方向回転によるXYZの移動(キーボード)
			RotKeyboard(isLimit);
		}
	}
	else
	{
		// 方向回転によるXYZの移動(ゲームパッド)
		RotGamePad(isLimit);
	}

}

void Camera::ProcessMove(void)
{

	auto& ins = InputManager::GetInstance();

	VECTOR moveDir = AsoUtility::VECTOR_ZERO;

	if (GetJoypadNum() == 0)
	{
		if (ins.IsNew(KEY_INPUT_W)) { moveDir = AsoUtility::DIR_F; }
		if (ins.IsNew(KEY_INPUT_S)) { moveDir = AsoUtility::DIR_B; }
		if (ins.IsNew(KEY_INPUT_A)) { moveDir = AsoUtility::DIR_L; }
		if (ins.IsNew(KEY_INPUT_D)) { moveDir = AsoUtility::DIR_R; }
	}
	else
	{

		InputManager::JOYPAD_IN_STATE padState =
			ins.GetJPadInputState(InputManager::JOYPAD_NO::PAD1);

		// 左スティックの傾き
		moveDir = ins.GetDirectionXZAKey(padState.AKeyLX, padState.AKeyLY);

	}

	// 移動処理
	if (!AsoUtility::EqualsVZero(moveDir))
	{

		// 移動させたい方向(ベクトル)に変換

		// 現在の向きからの進行方向を取得
		VECTOR direction = VNorm(transform_.quaRot.PosAxis(moveDir));

		// 移動させたい方向に移動量をかける(=移動量)
		VECTOR movePow = VScale(direction, SPEED);

		// カメラ位置も注視点も移動させる
		transform_.pos = VAdd(transform_.pos, movePow);
		targetPos_ = VAdd(targetPos_, movePow);

	}

}

void Camera::SetBeforeDrawFixedPoint(void)
{
	// 何もしない
}

void Camera::SetBeforeDrawFree(void)
{

	// カメラ操作(回転)
	ProcessRot(false);

	// カメラ操作(移動)
	ProcessMove();

	// Y軸
	rotY_ = Quaternion::AngleAxis(angles_.y, AsoUtility::AXIS_Y);

	// Y軸 + X軸
	transform_.quaRot = rotY_.Mult(Quaternion::AngleAxis(angles_.x, AsoUtility::AXIS_X));

	// 注視点更新
	targetPos_ = VAdd(transform_.pos, transform_.quaRot.PosAxis(FOLLOW_TARGET_LOCAL_POS));

	// カメラの上方向更新
	transform_.quaRot.GetUp();

}

void Camera::SetBeforeDrawFollow(void)
{

	// カメラ操作(回転)
	ProcessRot(true);

	// カメラ操作
	ProcessZoom();

	if (isZooming_ && zoomTimer_ > 0.0f)
	{
		// ズームタイマーの更新
		zoomTimer_ -= 1.0f / 60.0f; // スローモーション中ですが、実時間で減算されるように設定

		// 同期処理を一度実行してベースとなるカメラ位置を作る
		SyncFollow();

		// ズーム演出：ターゲット座標（接触点）にカメラと注視点を引き寄せる
		// 接触ポイントの少し斜め上空にカメラを回り込ませる
		VECTOR zoomCameraOffset = { 0.0f, 150.0f, -200.0f }; // コマの接触点を近くで映すためのオフセット（お好みで調整）
		VECTOR idealZoomCameraPos = VAdd(zoomTargetPoint_, zoomCameraOffset);

		// 線形補間（Lerp）を用いて、通常カメラの位置からズーム位置へとスムーズに遷移させる
		// LERP_RATE_MOVE (0.1f) よりも少し早く寄せるために高めのブレンド率にします
		transform_.pos = AsoUtility::Lerp(transform_.pos, idealZoomCameraPos, 0.4f);
		targetPos_ = AsoUtility::Lerp(targetPos_, zoomTargetPoint_, 0.4f);

		if (zoomTimer_ <= 0.0f)
		{
			isZooming_ = false;
		}
	}
	else
	{
		// 通常の追従処理
		SyncFollow();
	}

	// 衝突判定
	Collision();

	if (isCollision_)
	{
		// カメラ位置の補間
		transform_.pos =
			AsoUtility::Lerp(prePos_, transform_.pos, LERP_RATE_MOVE);
	}

}

void Camera::Collision(void)
{
	// プレイヤーのルートフレーム
	VECTOR start = MV1GetFramePosition(followTransform_->modelId, 1);
	isCollision_ = false;

	for (const auto& hitCol : hitColliders_)
	{
		// モデル以外は処理を飛ばす
		if (hitCol->GetShape() != ColliderBase::SHAPE::MODEL) continue;

		// 派生クラスへキャスト
		const ColliderModel* colliderModel =
			dynamic_cast<const ColliderModel*>(hitCol);

		if (colliderModel == nullptr) continue;

		auto hitPoly = colliderModel->GetNearestHitPolyLine(start, transform_.pos, false, true);

		if (!hitPoly.HitFlag)
		{
			isCollision_ = true;

			// 衝突していなければ次のコライダへ
			continue;
		}

		// カメラ位置から注視点への方向
		VECTOR dirToTarget = VNorm(VSub(targetPos_, transform_.pos));

		// 衝突点の少し手前にカメラを置く
		transform_.pos =
			VAdd(hitPoly.HitPosition, VScale(dirToTarget, COLLISION_BACK_DIS));

		// カメラ位置の球体コライダ
		int typeSphere = static_cast<int>(COLLIDER_TYPE::SPHERE);

		// 球体コライダが無ければ処理を抜ける
		if (ownColliders_.count(typeSphere) == 0) continue;

		if (ownColliders_.at(typeSphere) == nullptr) return;

		transform_.pos = ownColliders_.at(typeSphere)->
			GetPosPushBackAlongNormal(hitPoly, CNT_TRY_COLLISION_CAMERA, COLLISION_BACK_DIS);
	}
}

void Camera::RotKeyboard(bool isLimit)
{

	const auto& ins = InputManager::GetInstance();

	// カメラ回転
	if (ins.IsNew(KEY_INPUT_RIGHT))
	{
		// 右回転
		angles_.y += ROT_POW_RAD;
	}
	if (ins.IsNew(KEY_INPUT_LEFT))
	{
		// 左回転
		angles_.y -= ROT_POW_RAD;
	}

	// 上回転
	if (ins.IsNew(KEY_INPUT_UP))
	{
		angles_.x += ROT_POW_RAD;
		if (isLimit && angles_.x > LIMIT_X_UP_RAD)
		{
			angles_.x = LIMIT_X_UP_RAD;
		}
	}

	// 下回転
	if (ins.IsNew(KEY_INPUT_DOWN))
	{
		angles_.x -= ROT_POW_RAD;
		if (isLimit && angles_.x < -LIMIT_X_DW_RAD)
		{
			angles_.x = -LIMIT_X_DW_RAD;
		}
	}

}

void Camera::RotGamePad(bool isLimit)
{

	auto& ins = InputManager::GetInstance();

	// 接続されているゲームパッド１の情報を取得
	InputManager::JOYPAD_IN_STATE padState =
		ins.GetJPadInputState(InputManager::JOYPAD_NO::PAD1);

	// 右スティックの傾き
	VECTOR dir = ins.GetDirectionXZAKey(padState.AKeyRX, padState.AKeyRY);

	// 右スティック左右の傾き
	angles_.y += dir.x * ROT_POW_RAD;

	// 右スティック上下の傾き
	angles_.x += dir.z * ROT_POW_RAD;

	// 角度制限
	if (isLimit && angles_.x < -LIMIT_X_DW_RAD)
	{
		angles_.x = -LIMIT_X_DW_RAD;
	}
	if (isLimit && angles_.x > LIMIT_X_UP_RAD)
	{
		angles_.x = LIMIT_X_UP_RAD;
	}

}

void Camera::RotMouse(bool isLimit)
{
	auto& ins = InputManager::GetInstance();

	const int CENTER_X = Application::SCREEN_SIZE_X / 2;
	const int CENTER_Y = Application::SCREEN_SIZE_Y / 2;

	int mouseX, mouseY;
	GetMousePoint(&mouseX, &mouseY);

	// 画面中央からの移動量を直接計算
	float deltaX = static_cast<float>(mouseX - CENTER_X);
	float deltaY = static_cast<float>(mouseY - CENTER_Y);

	// 移動量がほぼ 0 でなければカメラの角度を更新
	if (deltaX != 0.0f || deltaY != 0.0f)
	{
		// X軸
		angles_.x += deltaY * MOUSE_SENSITIVITY;

		// Y軸
		angles_.y += deltaX * MOUSE_SENSITIVITY;
	}
	// 角度制限（上下回転）
	if (isLimit)
	{
		if (angles_.x > LIMIT_X_UP_RAD)
		{
			angles_.x = LIMIT_X_UP_RAD;
		}
		if (angles_.x < -LIMIT_X_DW_RAD)
		{
			angles_.x = -LIMIT_X_DW_RAD;
		}
	}
	
	SetMousePoint(CENTER_X, CENTER_Y);
}
