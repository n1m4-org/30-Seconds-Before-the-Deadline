#pragma once

#include <memory>
#include <array>
#include <drawable/sprite/Sprite.h>

class InGameUI
{
public:
    InGameUI() {};
    ~InGameUI() {};

    /// <summary>
    /// 初期化 (スプライトの生成と初期レイアウト設定)
    /// </summary>
    void Initialize();

    /// <summary>
    /// 更新
    /// </summary>
    /// <param name="progress">pcへのデータ送信進捗</param>
    /// <param name="remainingTime">ステージの残り制限時間(秒)</param>
    void Update(float progress, float remainingTime);

    /// <summary>
    /// 描画
    /// </summary>
    void Draw();

private:
	enum SpriteIndex
	{
		kProgressBar,           // 1. 進捗バー塗りつぶし
		kProgressBarFrame,      // 2. 進捗バー枠線
		kProgressBarText,       // 3. 進捗バー上のテキスト
		kTimelimitText,         // 4. 制限時間テキスト
		kMoveExplanationText,   // 5. 移動説明テキスト
		kClearText,             // 6. クリアテキスト
		kSpriteCount
	};
	std::array<std::unique_ptr<Sprite>, kSpriteCount> pUISprites_; // !< UIスプライト

	std::unique_ptr<Sprite> pTimerTens_ = nullptr; // !< 制限時間(十の位)
	std::unique_ptr<Sprite> pTimerOnes_ = nullptr; // !< 制限時間(一の位)

	float barWidth_ = 400.0f; // 進捗バーの最大幅

	float progress_ = 0.0f; // pcへのデータ送信進捗
	float remainingTime_ = 30.0f; // 残り制限時間
};

