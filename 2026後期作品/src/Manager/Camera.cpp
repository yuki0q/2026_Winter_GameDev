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
#include <map>
#include <string>
#include "CsvLoader.h"
#include "Camera.h"

namespace
{
	// 角度(rad)を、最も近い回り方で目標へ近づける（-π～π をまたいでも遠回りしない）
	float LerpAngle(float from, float to, float rate)
	{
		float diff = to - from;
		while (diff > DX_PI_F) { diff -= DX_TWO_PI_F; }
		while (diff < -DX_PI_F) { diff += DX_TWO_PI_F; }
		return from + diff * rate;
	}

	// CameraShot.csv の returnDir 列（SHORTEST / RIGHT / LEFT）を回り方に変換
	Camera::RETURN_DIR ParseReturnDir(const std::string& str)
	{
		std::string upper = str;
		for (auto& c : upper)
		{
			if (c >= 'a' && c <= 'z') { c = static_cast<char>(c - 'a' + 'A'); }
		}

		if (upper == "RIGHT" || upper == "R") { return Camera::RETURN_DIR::RIGHT; }
		if (upper == "LEFT" || upper == "L") { return Camera::RETURN_DIR::LEFT; }
		return Camera::RETURN_DIR::SHORTEST;
	}

	// 2つの角度(rad)の差の絶対値（最も近い回り方）
	float AngleDiffAbs(float a, float b)
	{
		float diff = b - a;
		while (diff > DX_PI_F) { diff -= DX_TWO_PI_F; }
		while (diff < -DX_PI_F) { diff += DX_TWO_PI_F; }
		return fabsf(diff);
	}
}

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

	// 追従対象が変わったら、演出は打ち切る
	eventMode_ = EVENT_MODE::NONE;
}

void Camera::TriggerZoomIn(const VECTOR& zoomPoint1, const VECTOR& zoomPoint2, float duration)
{
	isZooming_ = true;
	zoomTargetPoint_ = AsoUtility::Lerp(zoomPoint1, zoomPoint2, 0.15);
	zoomTimer_ = duration;
}

Camera::EventShot Camera::GetShot(EVENT_SHOT shot)
{

	const float DEG = DX_PI_F / 180.0f;

	// 名前（CameraShot.csv の shot 列）と、CSV が読めなかった時の既定値
	// { yaw, pitch, dist, hold, blendIn, blendOut }
	const char* name = "FRONT";
	EventShot def = { 180.0f * DEG, 8.0f * DEG, 300.0f, -1.0f, 0.12f, 0.12f };

	switch (shot)
	{
	case EVENT_SHOT::PARRY:
		name = "PARRY";
		def = { 110.0f * DEG, 4.0f * DEG, 260.0f, 0.6f, 0.28f, 0.12f };
		break;
	case EVENT_SHOT::SLASH:
		name = "SLASH";
		def = { 80.0f * DEG, 3.0f * DEG, 450.0f, 0.9f, 0.18f, 0.10f };
		break;
	default:
		break;
	}

	// CSVは初回だけ読み込む
	// shot,yawDeg,pitchDeg,dist,hold,blendIn,blendOut
	static bool isLoaded = false;
	static std::map<std::string, EventShot> table;

	if (!isLoaded)
	{
		isLoaded = true;

		for (const auto& r : CsvLoader::Load(Application::PATH_CSV + "CameraShot.csv"))
		{
			if (r.size() < 7)
			{
				continue;
			}

			EventShot s;
			s.yaw = CsvLoader::ToFloat(r[1], 0.0f) * DEG;
			s.pitch = CsvLoader::ToFloat(r[2], 0.0f) * DEG;
			s.dist = CsvLoader::ToFloat(r[3], 300.0f);
			s.hold = CsvLoader::ToFloat(r[4], 0.5f);
			s.blendIn = CsvLoader::ToFloat(r[5], 0.15f);
			s.blendOut = CsvLoader::ToFloat(r[6], 0.12f);

			// 戻る時の回り方（8列目。省略した場合は近いほう）
			s.returnDir = (r.size() >= 8) ? ParseReturnDir(r[7]) : RETURN_DIR::SHORTEST;

			table[r[0]] = s;
		}
	}

	auto it = table.find(name);
	return it != table.end() ? it->second : def;

}

void Camera::StartEventShot(EVENT_SHOT shot, RETURN_DIR returnDir)
{

	if (mode_ != MODE::FOLLOW || followTransform_ == nullptr)
	{
		return;
	}

	// 「終了を指示するまで続く」演出（必殺技の準備など）の間は、他の演出で上書きしない
	if (eventMode_ == EVENT_MODE::SHOT && currentShot_.hold < 0.0f)
	{
		return;
	}

	// 演出を始める前のカメラの向きを覚える
	// （演出中・戻り中に次の演出が始まった場合は、最初に覚えた向きを戻り先として維持する）
	if (eventMode_ == EVENT_MODE::NONE)
	{
		savedAngles_ = angles_;
		savedFollowDist_ = followDist_;
	}

	currentShot_ = GetShot(shot);

	// 戻る時の回り方：呼び出し側の指定があればそれを、DEFAULT なら CameraShot.csv の値を使う
	returnDir_ = (returnDir == RETURN_DIR::DEFAULT) ? currentShot_.returnDir : returnDir;


	// 演出のカメラ位置は、開始時のプレイヤーの向きを基準に固定する
	// （演出中にプレイヤーが向きを変えても、カメラが振り回されないようにする）
	VECTOR forward = followTransform_->quaRot.PosAxis(AsoUtility::DIR_F);
	forward.y = 0.0f;
	if (VSize(forward) < 0.001f)
	{
		forward = AsoUtility::DIR_F;
	}
	shotBaseYaw_ = atan2f(forward.x, forward.z);

	shotTimer_ = 0.0f;
	eventMode_ = EVENT_MODE::SHOT;
	
}

void Camera::StartFrontView(void)
{
	StartEventShot(EVENT_SHOT::FRONT);
}

void Camera::EndFrontView(void)
{
	// 終了を指示するまで続く演出の時だけ、元の向きへ戻す段階に移る
	if (eventMode_ == EVENT_MODE::SHOT && currentShot_.hold < 0.0f)
	{
		BeginReturn();
	}
}

void Camera::BeginReturn(void)
{
	eventMode_ = EVENT_MODE::RETURNING;

	// 今の角度から戻り先の角度までの差を、0 以上 2π 未満（正の向き = 右回り）に正規化する
	float diff = fmodf(savedAngles_.y - angles_.y, DX_TWO_PI_F);
	if (diff < 0.0f)
	{
		diff += DX_TWO_PI_F;
	}

	// すでに戻り先にいる場合に、誤差で1周してしまわないようにする
	if (diff < EVENT_END_EPSILON_RAD || diff > DX_TWO_PI_F - EVENT_END_EPSILON_RAD)
	{
		diff = 0.0f;
	}

	switch (returnDir_)
	{
	case RETURN_DIR::RIGHT:
		// 右回り（角度が増える向き）：そのまま
		break;

	case RETURN_DIR::LEFT:
		// 左回り（角度が減る向き）
		if (diff > 0.0f)
		{
			diff -= DX_TWO_PI_F;
		}
		break;

	default:
		// 近いほう
		if (diff > DX_PI_F)
		{
			diff -= DX_TWO_PI_F;
		}
		break;
	}

	returnTargetY_ = angles_.y + diff;
}

void Camera::UpdateEvent(void)
{

	if (followTransform_ == nullptr)
	{
		eventMode_ = EVENT_MODE::NONE;
		return;
	}

	// マウスは中央に戻し続ける（演出後に溜まった移動量で、カメラが急に回らないようにする）
	DiscardMouseMove();

	// 時間で終わる演出は、経過したら元の向きへ戻す段階へ
	// （ヒットストップなどでゲームが止まっても、カメラの時間は実時間で進む）
	if (eventMode_ == EVENT_MODE::SHOT && currentShot_.hold >= 0.0f)
	{
		shotTimer_ += sceMng_.GetDeltaTime();
		if (shotTimer_ >= currentShot_.hold)
		{
			BeginReturn();
		}
	}
	
	float targetX = 0.0f;
	float targetDist = 0.0f;
	float rate = 0.0f;

	if (eventMode_ == EVENT_MODE::SHOT)
	{
		// 開始時のプレイヤーの向きから、yaw だけ回り込んだ位置へ寄せる
		float targetY = shotBaseYaw_ + currentShot_.yaw;
		targetX = currentShot_.pitch;
		targetDist = currentShot_.dist;
		rate = currentShot_.blendIn;

		angles_.y = LerpAngle(angles_.y, targetY, rate);
	}
	else
	{
		// 演出前の向き・距離へ戻す
		// 向きは、戻り始めに決めた目標の角度へ向かう（右回り・左回りの指定を守るため、毎フレームは近道を選び直さない）
		targetX = savedAngles_.x;
		targetDist = savedFollowDist_;
		rate = currentShot_.blendOut;

		angles_.y += (returnTargetY_ - angles_.y) * rate;
	}

	angles_.x += (targetX - angles_.x) * rate;
	followDist_ += (targetDist - followDist_) * rate;

	// 戻りきったら、元の値にそろえて通常のカメラ操作へ
	if (eventMode_ == EVENT_MODE::RETURNING &&
		AngleDiffAbs(angles_.y, savedAngles_.y) < EVENT_END_EPSILON_RAD &&
		fabsf(angles_.x - savedAngles_.x) < EVENT_END_EPSILON_RAD &&
		fabsf(followDist_ - savedFollowDist_) < EVENT_END_EPSILON_DIST)
	{
		angles_ = savedAngles_;
		followDist_ = savedFollowDist_;
		eventMode_ = EVENT_MODE::NONE;
	}

}

void Camera::DiscardMouseMove(void)
{
	// マウスが画面中央に固定されている間（左Alt未押下）だけ、中央へ戻す
	auto& ins = InputManager::GetInstance();
	if (ins.IsNew(KEY_INPUT_LALT))
	{
		return;
	}

	SetMousePoint(Application::SCREEN_SIZE_X / 2, Application::SCREEN_SIZE_Y / 2);
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
	if (isZooming_) return;
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

	// 演出もリセット
	eventMode_ = EVENT_MODE::NONE;
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
	if (isZooming_) return;

	auto& ins = InputManager::GetInstance();

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

	// 方向回転によるXYZの移動(ゲームパッド)
	RotGamePad(isLimit);

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
	if (eventMode_ == EVENT_MODE::NONE)
	{
		// カメラ操作(回転)
		ProcessRot(true);

		// カメラ操作
		ProcessZoom();
	}
	else
	{
		// 演出中はカメラ操作を受け付けず、演出用にカメラの向きを動かす
		UpdateEvent();
	}

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
