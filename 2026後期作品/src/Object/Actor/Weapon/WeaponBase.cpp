#include "WeaponBase.h"

std::string WeaponTypeToName(WEAPON_TYPE type)
{
    switch (type)
    {
    case WEAPON_TYPE::SWORD:       return "SWORD";
    case WEAPON_TYPE::SHIELD:      return "SHIELD";
    case WEAPON_TYPE::GREAT_SWORD: return "GREAT_SWORD";
    case WEAPON_TYPE::AXE:         return "E_AXE";
    default:                       return "NONE";
    }
}

WEAPON_TYPE NameToWeaponType(const std::string& name)
{
    if (name == "SWORD")       return WEAPON_TYPE::SWORD;
    if (name == "SHIELD")      return WEAPON_TYPE::SHIELD;
    if (name == "GREAT_SWORD") return WEAPON_TYPE::GREAT_SWORD;
    if (name == "E_AXE")       return WEAPON_TYPE::AXE;
    return WEAPON_TYPE::NONE;
}

WeaponBase::WeaponBase(void)
    : type_(WEAPON_TYPE::NONE),
     weaponModelId_(-1),
     attachFrameIndex_(-1),
     modelHandle_(-1),
     localPos_{ 0,0,0 },
     localRot_{ 0,0,0 },
     scale_{ 1,1,1 },
     isAttackEnable_(false)
{
}

WeaponBase::~WeaponBase(void)
{
}

void WeaponBase::Init(int modelId, int attachFrameIndex, int modelHandle,
    const VECTOR& scale, const VECTOR& localPos, const VECTOR& localRot)
{
    // 派生クラスのコンストラクタで type_ が決まった後なので、ここでCSVから取得
    param_ = PlayerDataTable::GetWeaponParam(WeaponTypeToName(type_));

    weaponModelId_ = modelId;
    attachFrameIndex_ = attachFrameIndex;
    modelHandle_ = modelHandle;
    scale_ = scale;
    localPos_ = localPos;
    localRot_ = localRot;

    transform_.SetModel(modelHandle_); // 3dモデルを紐づけ
}

void WeaponBase::Update(void)
{
    if (weaponModelId_ == -1 || attachFrameIndex_ == -1 || modelHandle_ == -1) return;

    // 武器ローカル変換行列の計算
    MATRIX scaleMat = MGetScale(scale_);

    MATRIX localRotMat = MGetRotX(localRot_.x);
    localRotMat = MMult(localRotMat, MGetRotY(localRot_.y));
    localRotMat = MMult(localRotMat, MGetRotZ(localRot_.z));

    MATRIX localPosMat = MGetTranslate(localPos_);

    // 武器ローカル変換行列の合成（スケール -> 回転 -> 移動）
    MATRIX weaponLocalMat = MMult(scaleMat, localRotMat);
    weaponLocalMat = MMult(weaponLocalMat, localPosMat);

    // 所有者ボーンのワールド行列を取得して最終合成
    MATRIX frameWorldMat = MV1GetFrameLocalWorldMatrix(weaponModelId_, attachFrameIndex_);
    MATRIX finalMat = MMult(weaponLocalMat, frameWorldMat);

    // モデルに行列を直接反映（transform_.Update() は使わない）
    MV1SetMatrix(modelHandle_, finalMat);

    // コライダ追従用に Transform の位置と回転情報のみ保持
    transform_.pos = VGet(finalMat.m[3][0], finalMat.m[3][1], finalMat.m[3][2]);
    transform_.quaRot = Quaternion::GetRotation(finalMat);
    transform_.scl = scale_;

    //auto& ins = InputManager::GetInstance();
       //// ゲームパッドが接続されている数で処理を分ける
       //if (GetJoypadNum() == 0)
       //{
       //    // WASDで移動する
       //    if (ins.IsNew(KEY_INPUT_Q)) { transform_.pos.y += 0.01f; }
       //    if (ins.IsNew(KEY_INPUT_E)) { transform_.pos.y -= 0.01f; }
       //    if (ins.IsNew(KEY_INPUT_R)) { transform_.pos.z -= 0.01f; }
       //    if (ins.IsNew(KEY_INPUT_Z)) { transform_.pos.z += 0.01f; }
       //    if (ins.IsNew(KEY_INPUT_X)) { transform_.pos.x -= 0.01f; }
       //    if (ins.IsNew(KEY_INPUT_C)) { transform_.pos.x += 0.01f; }

       //}
}

void WeaponBase::Draw(void)
{
    if (modelHandle_ != -1)
    {
        MV1DrawModel(modelHandle_);
    }
}

void WeaponBase::DrawDebug(void)
{
    // 攻撃判定がONのときデバッグ表示を描画
    if (collider_)
    {
        collider_->Draw();
    }
}

void WeaponBase::Release(void)
{
    collider_.reset();
}