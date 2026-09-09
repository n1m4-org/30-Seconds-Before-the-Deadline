#include "TitleMenu.h"
#include <Core/Window/Window.h>
#include <Core/DirectX12/TextureManager.h>
#include <config/ResourcePath.h>
#include <dinput.h>
#include <Xinput.h>
#include <cmath>
#include <algorithm>
#include <Features/Audio/AudioManager.h>

TitleMenu::TitleMenu()
{
}

TitleMenu::~TitleMenu()
{
}

void TitleMenu::Initialize()
{
    TextureManager* tm = TextureManager::GetInstance();
    tm->LoadTexture(Path::Image::InGame::kTestTile); // Simple.png
    tm->LoadTexture(Path::Image::InGame::kStartGame); // goGame_text.png
    tm->LoadTexture(Path::Image::InGame::kEndGame);   // exitGame_text.png

    // SE初期化
    pChoiceAudio_ = AudioManager::GetInstance()->GetNewAudio("SE", Path::Audio::kChoiceSE);
    if (pChoiceAudio_)
    {
        pChoiceAudio_->SetVolume(0.12f);
    }
    pDecisionAudio_ = AudioManager::GetInstance()->GetNewAudio("SE", Path::Audio::kDecisionSE);
    if (pDecisionAudio_)
    {
        pDecisionAudio_->SetVolume(0.18f);
    }

    // 1. ボタン背景プレート
    for (int i = 0; i < kItemCount; ++i)
    {
        pButtonBgSprites_[i] = std::make_unique<Sprite>();
        pButtonBgSprites_[i]->Initialize(Path::Image::InGame::kTestTile);
        pButtonBgSprites_[i]->SetAnchorPoint({ 0.5f, 0.5f });
    }

    // 2. ボタンテキストスプライト (512x128)
    pButtonTextSprites_[kStartGame] = std::make_unique<Sprite>();
    pButtonTextSprites_[kStartGame]->Initialize(Path::Image::InGame::kStartGame);
    pButtonTextSprites_[kStartGame]->SetAnchorPoint({ 0.5f, 0.5f });

    pButtonTextSprites_[kEndGame] = std::make_unique<Sprite>();
    pButtonTextSprites_[kEndGame]->Initialize(Path::Image::InGame::kEndGame);
    pButtonTextSprites_[kEndGame]->SetAnchorPoint({ 0.5f, 0.5f });

    // 3. 選択中カーソル外枠ハイライト
    pCursorBorderSprite_ = std::make_unique<Sprite>();
    pCursorBorderSprite_->Initialize(Path::Image::InGame::kTestTile);
    pCursorBorderSprite_->SetAnchorPoint({ 0.5f, 0.5f });

    // 4. 選択中左右インジケーターバー
    pIndicatorLeft_ = std::make_unique<Sprite>();
    pIndicatorLeft_->Initialize(Path::Image::InGame::kTestTile);
    pIndicatorLeft_->SetAnchorPoint({ 0.5f, 0.5f });

    pIndicatorRight_ = std::make_unique<Sprite>();
    pIndicatorRight_->Initialize(Path::Image::InGame::kTestTile);
    pIndicatorRight_->SetAnchorPoint({ 0.5f, 0.5f });

    selectedIndex_ = kStartGame;
    currentAction_ = TitleMenuAction::None;
    animTimer_ = 0.0f;

    UpdateLayout();
}

void TitleMenu::TriggerActionByIndex(int index)
{
    if (index == kStartGame)
    {
        currentAction_ = TitleMenuAction::StartGame;
    }
    else if (index == kEndGame)
    {
        currentAction_ = TitleMenuAction::EndGame;
    }
}

TitleMenuAction TitleMenu::ConsumeAction()
{
    TitleMenuAction action = currentAction_;
    currentAction_ = TitleMenuAction::None;
    return action;
}

void TitleMenu::Update(Input* pInput, InputMapper<InputActionUI>* pInputMapper)
{
    animTimer_ += 0.05f;

    if (pInput)
    {
        int prevSelected = selectedIndex_;

        // 1. キーボード・コントローラー上下移動 (↑ / W / 十字上)
        bool moveUp = pInput->TriggerKey(DIK_UP) || pInput->TriggerKey(DIK_W);
        bool moveDown = pInput->TriggerKey(DIK_DOWN) || pInput->TriggerKey(DIK_S);

        if (moveUp)
        {
            selectedIndex_ = (selectedIndex_ + kItemCount - 1) % kItemCount;
        }
        else if (moveDown)
        {
            selectedIndex_ = (selectedIndex_ + 1) % kItemCount;
        }

        // カーソル移動SE再生
        if (selectedIndex_ != prevSelected)
        {
            if (pChoiceAudio_)
            {
                pChoiceAudio_->Play();
            }
        }

        // 3. 決定入力 (Space, Enter, テンキーEnter)
        bool confirmTriggered = pInput->TriggerKey(DIK_SPACE) ||
                                pInput->TriggerKey(DIK_RETURN) ||
                                pInput->TriggerKey(DIK_NUMPADENTER);

        if (confirmTriggered)
        {
            if (pDecisionAudio_)
            {
                pDecisionAudio_->Play();
            }
            TriggerActionByIndex(selectedIndex_);
        }
    }

    UpdateLayout();
}

void TitleMenu::UpdateLayout()
{
    float screenW = static_cast<float>(Window::clientWidth > 0 ? Window::clientWidth : 1600);
    float centerX = screenW * 0.5f;

    const float btnYPositions[kItemCount] = { 510.0f, 660.0f };
    const float basePlateW = 460.0f;
    const float basePlateH = 115.0f;
    const float baseTextW = 420.0f;
    const float baseTextH = 105.0f;

    float pulse = 1.0f + 0.035f * std::sin(animTimer_ * 3.5f);

    for (int i = 0; i < kItemCount; ++i)
    {
        bool isSelected = (i == selectedIndex_);
        float posY = btnYPositions[i];

        // 背景プレート
        if (pButtonBgSprites_[i])
        {
            pButtonBgSprites_[i]->SetPosition({ centerX, posY });
            if (isSelected)
            {
                pButtonBgSprites_[i]->SetSize({ basePlateW * pulse, basePlateH * pulse });
                pButtonBgSprites_[i]->SetColor({ 0.18f, 0.32f, 0.55f, 0.92f }); // 洗練されたハイライトブルー
            }
            else
            {
                pButtonBgSprites_[i]->SetSize({ basePlateW, basePlateH });
                pButtonBgSprites_[i]->SetColor({ 0.09f, 0.11f, 0.18f, 0.68f }); // 落ち着いたダークトーン
            }
            pButtonBgSprites_[i]->Update();
        }

        // ボタンテキスト
        if (pButtonTextSprites_[i])
        {
            pButtonTextSprites_[i]->SetPosition({ centerX, posY });
            if (isSelected)
            {
                pButtonTextSprites_[i]->SetSize({ baseTextW * pulse, baseTextH * pulse });
                pButtonTextSprites_[i]->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
            }
            else
            {
                pButtonTextSprites_[i]->SetSize({ baseTextW, baseTextH });
                pButtonTextSprites_[i]->SetColor({ 0.65f, 0.70f, 0.80f, 0.65f });
            }
            pButtonTextSprites_[i]->Update();
        }
    }

    // 選択中のカーソル外枠ハイライト
    float selY = btnYPositions[selectedIndex_];
    if (pCursorBorderSprite_)
    {
        float borderW = (basePlateW + 12.0f) * pulse;
        float borderH = (basePlateH + 12.0f) * pulse;
        float glowAlpha = 0.55f + 0.30f * std::sin(animTimer_ * 4.0f);

        pCursorBorderSprite_->SetPosition({ centerX, selY });
        pCursorBorderSprite_->SetSize({ borderW, borderH });
        pCursorBorderSprite_->SetColor({ 0.35f, 0.75f, 1.0f, glowAlpha });
        pCursorBorderSprite_->Update();
    }

    // 左右アクセントバー
    const float indicatorDist = (basePlateW * 0.5f + 32.0f) * pulse;
    const Vector2 indicatorSize = { 10.0f, 48.0f * pulse };
    const Vector4 indicatorColor = { 0.45f, 0.88f, 1.0f, 0.95f };

    if (pIndicatorLeft_)
    {
        pIndicatorLeft_->SetPosition({ centerX - indicatorDist, selY });
        pIndicatorLeft_->SetSize(indicatorSize);
        pIndicatorLeft_->SetColor(indicatorColor);
        pIndicatorLeft_->Update();
    }

    if (pIndicatorRight_)
    {
        pIndicatorRight_->SetPosition({ centerX + indicatorDist, selY });
        pIndicatorRight_->SetSize(indicatorSize);
        pIndicatorRight_->SetColor(indicatorColor);
        pIndicatorRight_->Update();
    }
}

void TitleMenu::Draw()
{
    // 1. 選択中カーソル外枠
    if (pCursorBorderSprite_)
    {
        pCursorBorderSprite_->Draw1F();
    }

    // 2. ボタン背景プレート
    for (auto&& bg : pButtonBgSprites_)
    {
        if (bg)
        {
            bg->Draw1F();
        }
    }

    // 3. 左右インジケーターバー
    if (pIndicatorLeft_)
    {
        pIndicatorLeft_->Draw1F();
    }
    if (pIndicatorRight_)
    {
        pIndicatorRight_->Draw1F();
    }

    // 4. ボタンテキストスプライト
    for (auto&& text : pButtonTextSprites_)
    {
        if (text)
        {
            text->Draw1F();
        }
    }
}

