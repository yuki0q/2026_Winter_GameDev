#pragma once
#include "../ActorBase.h"

class UIBase
{
public:
	UIBase(void);
	virtual ~UIBase(void);

	virtual void Init(void) = 0;
	virtual void Update(void) = 0;
	virtual void Draw(void) = 0;
	virtual void Release(void) = 0;

	// 有効・無効判定
	bool IsActive(void) const;
	// 有効・無効設定
	void SetActive(bool active);
	// 更新ステップ
	float GetStep(void) const;

protected:
	// 画像のハンドルID
	int img_;
	// 有効・無効
	bool active_;
	// 更新ステップ
	float step_;
};