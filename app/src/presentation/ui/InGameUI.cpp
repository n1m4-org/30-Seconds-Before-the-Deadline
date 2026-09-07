#include "InGameUI.h"
#include <config/ResourcePath.h>

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
	pUISprites_[kTimelimitText]->SetPosition({ 1280.0f, 300.0f }); // 制限時間テキストの位置を設定
	pUISprites_[kTimelimitText]->SetSize(pUISprites_[kTimelimitText]->GetSize() / 3.0f); // 初期サイズを設定

	pUISprites_[kMoveExplanationText] = std::make_unique<Sprite>();
	pUISprites_[kMoveExplanationText]->Initialize(Path::Image::InGame::kMoveExplanationText);
	pUISprites_[kMoveExplanationText]->SetAnchorPoint({ 0.0f, 0.5f }); // 左端を基準にする
	pUISprites_[kMoveExplanationText]->SetPosition({ 50.0f, 500.0f }); // 移動説明テキストの位置を設定

	pUISprites_[kClearText] = std::make_unique<Sprite>();
	pUISprites_[kClearText]->Initialize(Path::Image::InGame::kClearText);
	pUISprites_[kClearText]->SetAnchorPoint({ 0.0f, 0.5f }); // 左端を基準にする
	pUISprites_[kClearText]->SetPosition({ 1300.0f, 400.0f }); // クリアテキストの位置を設定
	pUISprites_[kClearText]->SetSize(pUISprites_[kClearText]->GetSize() / 2.0f); // 初期サイズを設定
}

void InGameUI::Update(float progress)
{
	progress_ = progress;
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
			sprite->Draw1F();
		}
		index++;
	}
}
