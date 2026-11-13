#pragma once
#include <Windows.h>
#include <DxLib.h>

#pragma comment(lib, "winmm.lib")

// フレームレートの制限（CPUを使い切らずに、次のフレームまで待つ）
//
// 「目標の時刻になるまで、時刻を読み続ける」待ち方は、待っている間ずっと
// CPU を使い切る（QueryPerformanceCounter が大量に呼ばれる）。
// この class は、残り時間が長い間は Sleep で寝て、最後のごく短い間だけ精密に待つ。
//
// 使い方:
//   FrameLimiter limiter(60);          // ループの前に作る
//   while (ProcessMessage() == 0)
//   {
//       Update();
//       Draw();
//       ScreenFlip();
//       limiter.Wait();                // ループの最後で呼ぶ
//   }
//
// ※ 垂直同期と二重に待たないよう、使う時は SetWaitVSyncFlag(FALSE) にしておく。
class FrameLimiter
{

public:

	explicit FrameLimiter(int targetFps = 60)
		:
		frameUs_(1000000LL / (targetFps > 0 ? targetFps : 60)),
		nextTime_(0),
		lastEndTime_(0),
		avgFrameUs_(0.0f)
	{
		// Sleep の精度を 1ms にする（既定では約15msで、待ち時間がずれる）
		timeBeginPeriod(1);
	}

	~FrameLimiter(void)
	{
		timeEndPeriod(1);
	}

	// 次のフレームの開始時刻まで待つ（1フレームに1回、ループの最後で呼ぶ）
	void Wait(void)
	{

		LONGLONG now = GetNowHiPerformanceCount();

		// 最初の1回は、基準の時刻を決めるだけ
		if (nextTime_ == 0)
		{
			nextTime_ = now + frameUs_;
			lastEndTime_ = now;
			return;
		}

		// 残り時間が長い間は Sleep で寝て、CPUを使わずに待つ
		LONGLONG remain = nextTime_ - now;
		if (remain > SPIN_US)
		{
			Sleep(static_cast<DWORD>((remain - SPIN_US) / 1000));
		}

		// 最後の約2msだけ、正確さのために時刻を読んで待つ
		while (GetNowHiPerformanceCount() < nextTime_)
		{
			YieldProcessor();
		}

		nextTime_ += frameUs_;

		// 大きく遅れた時（シーン切替の読み込みなど）は、
		// 遅れを取り戻そうとして連続で走らないよう、基準を引き直す
		now = GetNowHiPerformanceCount();
		if (now > nextTime_ + frameUs_ * 2)
		{
			nextTime_ = now + frameUs_;
		}

		// 平均のフレーム時間を更新（表示用。直近を重く見る移動平均）
		float frameUs = static_cast<float>(now - lastEndTime_);
		avgFrameUs_ = (avgFrameUs_ <= 0.0f) ? frameUs : avgFrameUs_ * 0.95f + frameUs * 0.05f;
		lastEndTime_ = now;

	}

	// 平均FPS（表示用）
	float GetFps(void) const
	{
		return avgFrameUs_ > 0.0f ? 1000000.0f / avgFrameUs_ : 0.0f;
	}

private:

	// 最後に時刻を読んで待つ時間（マイクロ秒）
	static constexpr LONGLONG SPIN_US = 2000;

	// 1フレームの長さ（マイクロ秒）
	LONGLONG frameUs_;

	// 次のフレームの開始時刻（マイクロ秒）
	LONGLONG nextTime_;

	// 前回 Wait を抜けた時刻と、平均のフレーム時間（FPS表示用）
	LONGLONG lastEndTime_;
	float avgFrameUs_;

};
