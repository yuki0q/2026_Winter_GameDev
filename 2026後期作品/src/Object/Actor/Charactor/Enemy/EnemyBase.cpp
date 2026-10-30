#include "../../../../Utility/AsoUtility.h"
#include "../../../Collider/ColliderCapsule.h"
#include "../../Weapon/WeaponManager.h"
#include "../Player.h"
#include "EnemyBase.h"

EnemyBase::EnemyBase(ENEMY_TYPE type)
    : 
    enemyType_(type),
	currentState_(),
    searchAngleTimer_(),
    baseQuaRot_(),
    currentSearchQuaRot_(),
    isInitBaseForward_(false),
    attackStartFrameIndex_(),
    attackEndFrameIndex_(),
    attackLocalPosTop_({}),
    attackLocalPosDown_({})
{
    // 敵種別に対応するパラメータをCSVから取得（Init()より前に確定させる）
    param_ = EnemyDataTable::GetParam(TypeToName(type));

    // 攻撃判定に使うボーンのフレーム番号
    attackStartFrameIndex_ = param_.attackStartFrame;
    attackEndFrameIndex_ = param_.attackEndFrame;
}

std::string EnemyBase::TypeToName(ENEMY_TYPE type)
{
    switch (type)
    {
    case ENEMY_TYPE::ALTER:  return "ALTER";
    case ENEMY_TYPE::WIZARD: return "WIZARD";
    case ENEMY_TYPE::OGRE:   return "OGRE";
    default:                 return "";
    }
}

bool EnemyBase::NameToType(const std::string& name, ENEMY_TYPE& outType)
{
    for (int i = 0; i < static_cast<int>(ENEMY_TYPE::MAX); i++)
    {
        ENEMY_TYPE type = static_cast<ENEMY_TYPE>(i);
        if (TypeToName(type) == name)
        {
            outType = type;
            return true;
        }
    }
    return false;
}

EnemyBase::~EnemyBase(void)
{
}

void EnemyBase::Draw(void)
{
    if (isDead_) return;

    CharactorBase::Draw();

    // 武器を持つ敵は、武器も描画する
    if (weaponManager_)
    {
        weaponManager_->Draw();
    }

    if (!isDebug_)return;
    DrawDebug();
}

void EnemyBase::Release(void)
{
    if (weaponManager_)
    {
        weaponManager_->Release();
    }

	CharactorBase::Release();
}

void EnemyBase::UpdateProcess(void)
{
    if (isDead_) return;

	movePow_ = AsoUtility::VECTOR_ZERO;
	moveSpeed_ = 0.0f;

	// 現在のステートに応じた更新処理を実行
	switch (currentState_)
	{
	case STATE::IDLE:
		UpdateIdle();
		break;
	case STATE::DISCOVERY:
        UpdateDiscovery();
		break;
	case STATE::DAMAGE:
		UpdateDamage();
		break;
	case STATE::WALK:
		UpdateWalk();
		break;
	case STATE::RUN:
		UpdateRun();
		break;
	case STATE::PREV_ATTACK:
		UpdatePrevAttack();
        break;
	case STATE::ATTACK:
        SetAttackColliderEnable(true);
		UpdateAttack();
		break;
	case STATE::DIE:
		UpdateDie();
		break;
	case STATE::END:
		UpdateEnd();
		break;
	}

    // 攻撃ステート以外では攻撃判定を自動的にOFFにするなどの制御もここで可能
    if (currentState_ != STATE::ATTACK)
    {
        isAttackColliderEnable_ = false;
    }
}

void EnemyBase::UpdateProcessPost(void)
{
    if (isDead_) return;

    // アニメーション更新後に、武器をボーン(フレーム)の位置へ追従させる
    if (weaponManager_)
    {
        weaponManager_->Update();
    }
}

void EnemyBase::UpdateAttackCollider(void)
{
    // 攻撃判定が無効、またはフレームが未設定なら処理しない
    if (!isAttackColliderEnable_ || attackStartFrameIndex_ == -1 || attackEndFrameIndex_ == -1) return;

    if (!attackCollider_) return;

    // 1つ目のフレームのワールド行列を取得して位置を計算
    MATRIX startMat = MV1GetFrameLocalWorldMatrix(transform_.modelId, attackStartFrameIndex_);
    VECTOR startPos = VGet(startMat.m[3][0], startMat.m[3][1], startMat.m[3][2]);

    // 2つ目のフレームのワールド行列を取得して位置を計算
    MATRIX endMat = MV1GetFrameLocalWorldMatrix(transform_.modelId, attackEndFrameIndex_);
    VECTOR endPos = VGet(endMat.m[3][0], endMat.m[3][1], endMat.m[3][2]);

    // ColliderCapsule をキャストして、Top と Down のワールド座標（またはローカル座標）を直接更新する
    ColliderCapsule* capsuleCol = dynamic_cast<ColliderCapsule*>(attackCollider_.get());
    if (capsuleCol)
    {
        // ※ColliderCapsule側にワールド座標を直接セットする機能、あるいは
        //   Transformを原点にしてTop/Downをワールド座標で設定できるようにすると便利です。
        // ここでは例として、attackTransform_ を原点(0,0,0)にして、
        // Top と Down にそれぞれ取得したボーンのワールド座標を設定するアプローチをとります。

        attackTransform_.pos = AsoUtility::VECTOR_ZERO;
        attackTransform_.quaRot = Quaternion::Identity();
        attackTransform_.scl = AsoUtility::VECTOR_ONE;
        attackTransform_.Update();

        capsuleCol->SetLocalPosTop(startPos);
        capsuleCol->SetLocalPosDown(endPos);
    }

    //// キャラクターモデルの指定ボーン（フレーム）のワールド行列を取得
    //MATRIX frameWorldMat = MV1GetFrameLocalWorldMatrix(transform_.modelId, attackStartFrameIndex_);

    //// ボーンの位置と回転を、攻撃用コライダのTransform（あるいはコライダ自体）に反映させる
    //// ※攻撃用コライダが持っている、あるいは紐づいている Transform を更新する
    //if (attackCollider_)
    //{
    //    // コライダが参照しているTransformをボーンの位置・回転に合わせる
    //    // （ColliderBaseは follow_ へのポインタを持っているため、そこに渡しているTransformを更新します）

    //    // ※もし攻撃用コライダが独自の Transform を持っている場合は、そちらの pos / quaRot を更新します
    //    // 例：
    //    VECTOR bonePos = VGet(frameWorldMat.m[3][0], frameWorldMat.m[3][1], frameWorldMat.m[3][2]);
    //    Quaternion boneRot = Quaternion::GetRotation(frameWorldMat);

    //    // 例として、attackCollider_ が独自の Transform を持っている場合の更新イメージ
    //     attackTransform_.pos = bonePos;
    //     attackTransform_.quaRot = boneRot;
    //     attackTransform_.Update();
    //}
}

void EnemyBase::OnDamage(int damage, VECTOR attackedPos)
{
    if (isDead_ || (currentState_ == STATE::DAMAGE)) return;

    hp_ -= damage;
    if (hp_ <= 0)
    {
        hp_ = 0;
        ChangeState(STATE::DIE);
    }
    else
    {
        // ダメージを受けたときにDAMAGEステートに移行する
        ChangeState(STATE::DAMAGE);

        KnockBack(attackedPos, param_.knockbackPower, param_.knockbackTime);
    }
}

void EnemyBase::DrawDebug(void)
{
    // 攻撃判定が有効な場合、攻撃用コライダのデバッグ描画を行う
    if (isAttackColliderEnable_ && attackCollider_)
    {
        // ColliderBase側にDraw()がある前提
        attackCollider_->Draw();
    }

    // 所有しているコライダの描画
    for (const auto& own : ownColliders_)
    {
        own.second->Draw();
    }

    VECTOR pos = transform_.pos;

   // 索敵サークルの底面円を描画 (DxLibのDrawCircle3Dを使用)
    // 引数: 中心位置, 半径, 分割数, 色, 塗りつぶすか
    DrawSphere3D(pos, param_.searchRadius, 32, GetColor(0, 255, 0), GetColor(0, 255, 0), FALSE);

    VECTOR searchForward = currentSearchQuaRot_.PosAxis(AsoUtility::DIR_F);
    searchForward.y = 0.0f;
    searchForward = AsoUtility::VNormalize(searchForward);

    float halfAngleRad = AsoUtility::Deg2RadD(param_.searchViewAngle * 0.5f);

    // 左右の視界限界方向ベクトルを計算
    MATRIX rotLeftMat = MGetRotY(-halfAngleRad);
    MATRIX rotRightMat = MGetRotY(halfAngleRad);
    VECTOR leftDir = VTransform(searchForward, rotLeftMat);
    VECTOR rightDir = VTransform(searchForward, rotRightMat);

    // 視界の線を引く（中心から左右の端まで）
    VECTOR leftEnd = VAdd(pos, VScale(leftDir, param_.searchRadius));
    VECTOR rightEnd = VAdd(pos, VScale(rightDir, param_.searchRadius));

    // 高さを少し地面から浮かす（見やすさのため +10.0f）
    VECTOR drawPos = VAdd(pos, VGet(0.0f, 10.0f, 0.0f));
    VECTOR drawLeftEnd = VAdd(leftEnd, VGet(0.0f, 10.0f, 0.0f));
    VECTOR drawRightEnd = VAdd(rightEnd, VGet(0.0f, 10.0f, 0.0f));

    DrawLine3D(drawPos, drawLeftEnd, GetColor(255, 255, 0));
    DrawLine3D(drawPos, drawRightEnd, GetColor(255, 255, 0));
}

void EnemyBase::SetPosition(VECTOR spawnPos)
{
    transform_.pos = spawnPos;
    transform_.Update();
}

bool EnemyBase::SearchCircle(const VECTOR& playerPos) const
{
    // 敵とプレイヤーの距離を計算
    VECTOR diff = VSub(playerPos, transform_.pos);
    float distSq = VDot(diff, diff); // 2点間の距離の二乗

    // 半径の二乗と比較して効率化
    float radiusSq = param_.searchRadius * param_.searchRadius;
    if (distSq <= radiusSq)
    {
        return true; // 一定距離内に入っている
    }

    return false;
}


bool EnemyBase::SearchPlayer(const VECTOR& playerPos) const
{
    // 距離内に入っている
    if (!SearchCircle(playerPos))
    {
        return false;
    }

    VECTOR searchForward = currentSearchQuaRot_.PosAxis(AsoUtility::DIR_F);
    searchForward.y = 0.0f;
    searchForward = AsoUtility::VNormalize(searchForward);

    // 敵からプレイヤーへの方向ベクトルを計算
    VECTOR dirToPlayer = VSub(playerPos, transform_.pos);
    dirToPlayer.y = 0.0f;
    dirToPlayer = AsoUtility::VNormalize(dirToPlayer);

    // 内積から角度（コサイン値）を求める
    float dot = VDot(searchForward, dirToPlayer);

    // 視野角をラジアンに変換して、許容されるコサイン値と比較
    float halfAngleRad = AsoUtility::Deg2RadD(param_.searchViewAngle * 0.5f);
    float thresholdCos = cosf(halfAngleRad);

    if (dot >= thresholdCos)
    {
        return true; // 視界内にプレイヤーがいる
    }

    return false;
}

void EnemyBase::SerachOscillation(void)
{
    // 初回、またはIDLE移行時に基準の正面方向を保存しておく
    if (!isInitBaseForward_)
    {
        baseQuaRot_ = transform_.quaRot;
        currentSearchQuaRot_ = baseQuaRot_;
        isInitBaseForward_ = true;
    }

    // 走査の処理
    // 時間経過でタイマーを進める
    searchAngleTimer_ += 0.02f;

    // sin波を使って左右90度ずつの範囲で角度を往復させる
    float maxSwingDeg = 90.0f;
    float swingAngleRad = AsoUtility::Deg2RadF(maxSwingDeg) * sinf(searchAngleTimer_);

    Quaternion swingRot = Quaternion::AngleAxis(swingAngleRad, AsoUtility::AXIS_Y);

    // 基準の回転に、首振りのY軸回転を掛け合わせる（合成する）
    currentSearchQuaRot_ = baseQuaRot_.Mult(swingRot);
}

void EnemyBase::AfterChase(VECTOR pos)
{
    // 見失った瞬間のプレイヤーの方向を向かせる
    VECTOR lookDir = VSub(pos, transform_.pos);
    lookDir.y = 0.0f;
    if (VSize(lookDir) > 0.001f)
    {
        lookDir = AsoUtility::VNormalize(lookDir);

        // 方向ベクトルからY軸の回転角度（ラジアン）を求め、クォータニオンに変換
        float angle = atan2f(lookDir.x, lookDir.z);
        transform_.quaRot = Quaternion::AngleAxis(angle, AsoUtility::AXIS_Y);

        // 行列を更新してモデルに反映
        transform_.Update();
    }
}

void EnemyBase::InitFacing(void)
{
    // 向く方向は、プレイヤーのいる方向（水平方向のみ）
    VECTOR dir = AsoUtility::VECTOR_ZERO;
    if (player_)
    {
        dir = VSub(player_->GetTransform().pos, transform_.pos);
        dir.y = 0.0f;
    }

    // プレイヤーが未設定、または真上・真下などで方向が決まらない場合は +Z 方向
    if (VSize(dir) < 0.001f)
    {
        dir = AsoUtility::DIR_F;
    }
    dir = AsoUtility::VNormalize(dir);

    // moveDir_ も同じ向きにしておく
    // （DelayRotate は毎フレーム moveDir_ の向きへ回転を補間するため、
    //   初期値のゼロベクトルのままだと、設定した向きが別の向きへ回されてしまう）
    moveDir_ = dir;

    // quaRot は「ワールドでの向き」。モデル自体の向きの補正は quaRotLocal 側で行う
    transform_.quaRot = Quaternion::LookRotation(dir);

    // 索敵の基準方向も同じ向きにそろえておく
    baseQuaRot_ = transform_.quaRot;
    currentSearchQuaRot_ = transform_.quaRot;
}
