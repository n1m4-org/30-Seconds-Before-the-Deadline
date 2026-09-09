#include "InGameUI.h"
#include <config/ResourcePath.h>
#include <algorithm>
#include <cmath>

void InGameUI::Initialize()
{
	pUISprites_[kProgressBar] = std::make_unique<Sprite>();
	pUISprites_[kProgressBar]->Initialize(Path::Image::InGame::kProgressBar);
	pUISprites_[kProgressBar]->SetAnchorPoint({ 0.0f, 0.5f }); // 左端を基準にする
	pUISprites_[kProgressBar]->SetPosition({ 1300.0f, 500.0f }); // 進捗バーの位置を設定
	pUISprites_[kProgressBar]->SetSize(pUISprites_[kProgressBar]->GetSize() / 2.0f); // 初期サイズを設定
	barWidth_ = pUISprites_[kProgressBar]->GetSize().x; // 進捗バーの最大幅を取得

	pUISprites_[kProgressBarFrame] = std::make_unique<Sprite>();
	pUISprites_[kProgressBarFrame]->Initialize(Path::Image::InGame::kProgressBarFrame);
	pUISprites_[kProgressBarFrame]->SetAnchorPoint({ 0.0f, 0.5f }); // 左端を基準にする
	pUISprites_[kProgressBarFrame]->SetPosition({ 1300.0f, 500.0f }); // 進捗フレームの位置を設定
	pUISprites_[kProgressBarFrame]->SetSize(pUISprites_[kProgressBarFrame]->GetSize() / 2.0f); // 初期サイズを設定

	pUISprites_[kProgressBarText] = std::make_unique<Sprite>();
	pUISprites_[kProgressBarText]->Initialize(Path::Image::InGame::kProgressBarText);
	pUISprites_[kProgressBarText]->SetAnchorPoint({ 0.0f, 0.5f }); // 左端を基準にする
	pUISprites_[kProgressBarText]->SetPosition({ 1300.0f, 400.0f }); // 進捗テキストの位置を設定
	pUISprites_[kProgressBarText]->SetSize(pUISprites_[kProgressBarText]->GetSize() / 2.0f); // 初期サイズを設定

	pUISprites_[kTimelimitText] = std::make_unique<Sprite>();
	pUISprites_[kTimelimitText]->Initialize(Path::Image::InGame::kTimelimitText);
	pUISprites_[kTimelimitText]->SetAnchorPoint({ 0.0f, 0.5f }); // 左端を基準にする
	pUISprites_[kTimelimitText]->SetPosition({ 1270.0f, 300.0f }); // 制限時間テキストの位置を設定
	pUISprites_[kTimelimitText]->SetSize(pUISprites_[kTimelimitText]->GetSize() / 3.0f); // 初期サイズを設定

	// 制限時間表示用数字スプライト
	float timelimitW = pUISprites_[kTimelimitText]->GetSize().x;
	float timelimitH = pUISprites_[kTimelimitText]->GetSize().y;
	float timelimitTopY = 300.0f - timelimitH * 0.5f;
	float baselineY = timelimitTopY + (118.0f / 128.0f) * timelimitH; // 文字下端ライン

	Vector2 digitSize = { 48.0f, 64.0f }; // サイズ調整
	float digitTopY = baselineY - (118.0f / 128.0f) * digitSize.y;
	float digitY = digitTopY + digitSize.y * 0.5f; // AnchorPoint(0.0f, 0.5f) の中心Y座標
	float digitStartX = 1270.0f + timelimitW + 12.0f;

	pTimerTens_ = std::make_unique<Sprite>();
	pTimerTens_->Initialize(Path::Image::kUINumbers);
	pTimerTens_->SetAnchorPoint({ 0.0f, 0.5f });
	pTimerTens_->SetSize(digitSize);
	pTimerTens_->SetPosition({ digitStartX, digitY });
	pTimerTens_->SetTextureLeftTop({ 3.0f * 96.0f, 0.0f }); // 初期値3
	pTimerTens_->SetTextureSize({ 96.0f, 128.0f });

	pTimerOnes_ = std::make_unique<Sprite>();
	pTimerOnes_->Initialize(Path::Image::kUINumbers);
	pTimerOnes_->SetAnchorPoint({ 0.0f, 0.5f });
	pTimerOnes_->SetSize(digitSize);
	pTimerOnes_->SetPosition({ digitStartX + digitSize.x - 8.0f, digitY });
	pTimerOnes_->SetTextureLeftTop({ 0.0f * 96.0f, 0.0f }); // 初期値0
	pTimerOnes_->SetTextureSize({ 96.0f, 128.0f });

	pUISprites_[kMoveExplanationText] = std::make_unique<Sprite>();
	pUISprites_[kMoveExplanationText]->Initialize(Path::Image::InGame::kMoveExplanationText);
	pUISprites_[kMoveExplanationText]->SetAnchorPoint({ 0.0f, 0.5f }); // 左端を基準にする
	pUISprites_[kMoveExplanationText]->SetPosition({ 50.0f, 600.0f }); // 移動説明テキストの位置を設定

	pUISprites_[kClearText] = std::make_unique<Sprite>();
	pUISprites_[kClearText]->Initialize(Path::Image::InGame::kClearText);
	pUISprites_[kClearText]->SetAnchorPoint({ 0.0f, 0.5f }); // 左端を基準にする
	pUISprites_[kClearText]->SetPosition({ 1300.0f, 400.0f }); // クリアテキストの位置を設定
	pUISprites_[kClearText]->SetSize(pUISprites_[kClearText]->GetSize() / 2.0f); // 初期サイズを設定

	pUISprites_[kTutoriaText] = std::make_unique<Sprite>();
	pUISprites_[kTutoriaText]->Initialize(Path::Image::InGame::kTutoriaText);
	pUISprites_[kTutoriaText]->SetAnchorPoint({ 0.5f, 0.5f }); // 中心を基準にする
	pUISprites_[kTutoriaText]->SetPosition({ 800.0f, 150.0f }); // チュートリアルテキストの位置を設定
	pUISprites_[kTutoriaText]->SetSize(pUISprites_[kTutoriaText]->GetSize()); // 初期サイズを設定

	pUISprites_[kUndoExplanationText] = std::make_unique<Sprite>();
	pUISprites_[kUndoExplanationText]->Initialize(Path::Image::InGame::kUndoExplanationText);
	pUISprites_[kUndoExplanationText]->SetAnchorPoint({ 0.0f, 0.5f }); // 左端を基準にする
	pUISprites_[kUndoExplanationText]->SetPosition({ 20.0f, 390.0f }); // アンドゥ説明テキストの位置を設定
	pUISprites_[kUndoExplanationText]->SetSize(pUISprites_[kUndoExplanationText]->GetSize() / 1.5f); // 初期サイズを設定

	pUISprites_[kResetExplanationText] = std::make_unique<Sprite>();
	pUISprites_[kResetExplanationText]->Initialize(Path::Image::InGame::kResetExplanationText);
	pUISprites_[kResetExplanationText]->SetAnchorPoint({ 0.0f, 0.5f }); // 左端を基準にする
	pUISprites_[kResetExplanationText]->SetPosition({ 178.0f, 390.0f }); // リセット説明テキストの位置を設定
	pUISprites_[kResetExplanationText]->SetSize(pUISprites_[kResetExplanationText]->GetSize() / 1.5f); // 初期サイズを設定

	pUISprites_[kPauseExplanationText] = std::make_unique<Sprite>();
	pUISprites_[kPauseExplanationText]->Initialize(Path::Image::InGame::kPauseExplanationText);
	pUISprites_[kPauseExplanationText]->SetAnchorPoint({ 0.0f, 0.5f }); // 左端を基準にする
	pUISprites_[kPauseExplanationText]->SetPosition({ 8.0f, 60.0f }); // ポーズ説明テキストの位置を設定
	pUISprites_[kPauseExplanationText]->SetSize(pUISprites_[kPauseExplanationText]->GetSize() / 1.5f); // 初期サイズを設定
}

void InGameUI::Update(float progress, float remainingTime, bool isStage1_1)
{
	progress_ = progress;
	remainingTime_ = remainingTime;
	isStage1_1_ = isStage1_1;

	// 進捗バーのサイズを更新
	if (pUISprites_[kProgressBar])
	{
		float newWidth = barWidth_ * progress;
		pUISprites_[kProgressBar]->SetSize({ newWidth, pUISprites_[kProgressBar]->GetSize().y });

		// テクスチャの切り出しサイズも同じ比率で小さくする（左上は 0.0f で固定）
		float originalTextureWidth = barWidth_ * 2.0f;
		pUISprites_[kProgressBar]->SetTextureLeftTop({ 0.0f, 0.0f });
		pUISprites_[kProgressBar]->SetTextureSize({ originalTextureWidth * progress, pUISprites_[kProgressBar]->GetSize().y });
	}

	for (auto&& sprite : pUISprites_)
	{
		if (sprite)
		{
			sprite->Update();
		}
	}

	// 制限時間の数字スプライト更新 (0〜99秒)
	int displaySec = (std::clamp)(static_cast<int>(std::ceil(remainingTime_)), 0, 99);
	int tens = displaySec / 10;
	int ones = displaySec % 10;

	// 残り5秒以下なら赤系警告色
	Vector4 timerColor = (remainingTime_ <= 5.0f) ? Vector4{ 1.0f, 0.25f, 0.25f, 1.0f } : Vector4{ 1.0f, 1.0f, 1.0f, 1.0f };

	if (pTimerTens_)
	{
		pTimerTens_->SetTextureLeftTop({ static_cast<float>(tens) * 96.0f, 0.0f });
		pTimerTens_->SetTextureSize({ 96.0f, 128.0f });
		pTimerTens_->SetColor(timerColor);
		pTimerTens_->Update();
	}

	if (pTimerOnes_)
	{
		pTimerOnes_->SetTextureLeftTop({ static_cast<float>(ones) * 96.0f, 0.0f });
		pTimerOnes_->SetTextureSize({ 96.0f, 128.0f });
		pTimerOnes_->SetColor(timerColor);
		pTimerOnes_->Update();
	}
}

void InGameUI::Draw()
{
	int index = 0;
	for (auto&& sprite : pUISprites_)
	{
		if (sprite)
		{
			if (sprite == pUISprites_[kClearText] && progress_ < 1.0f)
			{
				index++;
				continue; // クリアテキストは条件付きで描画
			}
			if (sprite == pUISprites_[kProgressBarText] && (progress_ <= 0.0f || progress_ >= 1.0f))
			{
				index++;
				continue; // 進捗テキストは条件付きで描画
			}
			if (sprite == pUISprites_[kTutoriaText] && !isStage1_1_)
			{
				index++;
				continue; // チュートリアルテキストは1-1のみ表示
			}
			sprite->Draw1F();
		}
		index++;
	}

	// 制限時間数字の描画
	if (pTimerTens_)
	{
		pTimerTens_->Draw1F();
	}
	if (pTimerOnes_)
	{
		pTimerOnes_->Draw1F();
	}
}
