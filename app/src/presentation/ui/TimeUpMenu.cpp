#include "TimeUpMenu.h"
#include <Core/Window/Window.h>
#include <Core/DirectX12/TextureManager.h>
#include <config/ResourcePath.h>
#include <NiGui.h>
#include <dinput.h>
#include <cmath>
#include <algorithm>
#include <string>

TimeUpMenu::TimeUpMenu()
{
}

TimeUpMenu::~TimeUpMenu()
{
}

void TimeUpMenu::Initialize()
{
    TextureManager* tm = TextureManager::GetInstance();
    tm->LoadTexture(Path::Image::InGame::kTestTile); // game/tile/Simple.png をロード

    // 1. 全画面半透明オーバーレイ (時間切れの緊張感とドラマチックさを醸し出す赤みがかった暗幕)
    pOverlaySprite_ = std::make_unique<Sprite>();
    pOverlaySprite_->Initialize(Path::Image::InGame::kTestTile);
    pOverlaySprite_->SetAnchorPoint({ 0.5f, 0.5f });
    pOverlaySprite_->SetColor({ 0.12f, 0.02f, 0.03f, 0.72f });

    // 2. メニューパネル背景 (深みのあるダーククリムゾン/ワインレッド調)
    pPanelSprite_ = std::make_unique<Sprite>();
    pPanelSprite_->Initialize(Path::Image::InGame::kTestTile);
    pPanelSprite_->SetAnchorPoint({ 0.5f, 0.5f });
    pPanelSprite_->SetColor({ 0.16f, 0.07f, 0.09f, 0.95f });

    // 3. メニューボタン
    for (int i = 0; i < kItemCount; ++i)
    {
        pButtonSprites_[i] = std::make_unique<Sprite>();
        pButtonSprites_[i]->Initialize(Path::Image::InGame::kTestTile);
        pButtonSprites_[i]->SetAnchorPoint({ 0.5f, 0.5f });
    }
    pTextSprites_[0] = std::make_unique<Sprite>();
    pTextSprites_[0]->Initialize(Path::Image::InGame::kRetryText);
    pTextSprites_[0]->SetAnchorPoint({ 0.5f, 0.5f });

    pTextSprites_[1] = std::make_unique<Sprite>();
    pTextSprites_[1]->Initialize(Path::Image::InGame::kLetsGoSelectText);
    pTextSprites_[1]->SetAnchorPoint({ 0.5f, 0.5f });

    pTextSprites_[2] = std::make_unique<Sprite>();
    pTextSprites_[2]->Initialize(Path::Image::InGame::kLetsGoTitleText);
    pTextSprites_[2]->SetAnchorPoint({ 0.5f, 0.5f });

    // 4. カーソルインジケーター
    pCursorSprite_ = std::make_unique<Sprite>();
    pCursorSprite_->Initialize(Path::Image::InGame::kTestTile);
    pCursorSprite_->SetAnchorPoint({ 0.5f, 0.5f });
    pCursorSprite_->SetColor({ 1.0f, 0.9f, 0.7f, 0.95f });

    // 5. クリアテキスト
    pTimeUpSprite_ = std::make_unique<Sprite>();
    pTimeUpSprite_->Initialize(Path::Image::InGame::kDefeatText);
    pTimeUpSprite_->SetAnchorPoint({ 0.5f, 0.5f });
    pTimeUpSprite_->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });

    selectedIndex_ = kRetry;
    isOpen_ = false;
    currentAction_ = TimeUpMenuAction::None;
    animTimer_ = 0.0f;

    UpdateLayout();
}

void TimeUpMenu::Open()
{
    isOpen_ = true;
    selectedIndex_ = kRetry; // 開いた時は一番上の「リトライ」を選択
    currentAction_ = TimeUpMenuAction::None;
    animTimer_ = 0.0f;
    UpdateLayout();
}

void TimeUpMenu::Close()
{
    isOpen_ = false;
    currentAction_ = TimeUpMenuAction::None;
}

TimeUpMenuAction TimeUpMenu::ConsumeAction()
{
    TimeUpMenuAction action = currentAction_;
    currentAction_ = TimeUpMenuAction::None;
    return action;
}

void TimeUpMenu::TriggerActionByIndex(int index)
{
    switch (index)
    {
    case kRetry:
        currentAction_ = TimeUpMenuAction::Retry;
        break;
    case kStageSelect:
        currentAction_ = TimeUpMenuAction::StageSelect;
        break;
    case kTitle:
        currentAction_ = TimeUpMenuAction::Title;
        break;
    default:
        break;
    }
}

void TimeUpMenu::Update(Input* pInput)
{
    if (!isOpen_)
    {
        return;
    }

    animTimer_ += 0.03f;

    if (pInput)
    {
        // 1. キーボード移動 (W/S または 上下矢印)
        bool moveUp = pInput->TriggerKey(DIK_UP) || pInput->TriggerKey(DIK_W);
        bool moveDown = pInput->TriggerKey(DIK_DOWN) || pInput->TriggerKey(DIK_S);

        if (moveUp)
        {
            selectedIndex_ = (selectedIndex_ - 1 + kItemCount) % kItemCount;
        }
        if (moveDown)
        {
            selectedIndex_ = (selectedIndex_ + 1) % kItemCount;
        }

        // 2. マウスホバー & クリック判定
        /*float screenW = static_cast<float>(Window::clientWidth > 0 ? Window::clientWidth : 1600);
        float screenH = static_cast<float>(Window::clientHeight > 0 ? Window::clientHeight : 900);
        Vector2 center = { screenW * 0.5f, screenH * 0.5f };

        const float baseBtnW = 341.3f + 40.0f;
        const float baseBtnH = 85.3f;
        const float btnSpacing = 128.0f;
        const float startY = center.y - btnSpacing;

        POINT cursorPt = pInput->GetCursorPosition();
        float mouseX = static_cast<float>(cursorPt.x);
        float mouseY = static_cast<float>(cursorPt.y);

        bool mouseClicked = pInput->TriggerMouse(Input::MouseNum::Left);

        for (int i = 0; i < kItemCount; ++i)
        {
            float btnY = startY + static_cast<float>(i) * btnSpacing;
            float left = center.x - baseBtnW * 0.5f;
            float right = center.x + baseBtnW * 0.5f;
            float top = btnY - baseBtnH * 0.5f;
            float bottom = btnY + baseBtnH * 0.5f;

            if (mouseX >= left && mouseX <= right && mouseY >= top && mouseY <= bottom)
            {
                selectedIndex_ = i;
                if (mouseClicked)
                {
                    TriggerActionByIndex(i);
                    UpdateLayout();
                    return;
                }
            }
        }*/

        // 3. キーボード決定 (Space, Enter, テンキーEnter, Z キー)
        if (pInput->TriggerKey(DIK_SPACE) || 
            pInput->TriggerKey(DIK_RETURN) || 
            pInput->TriggerKey(DIK_NUMPADENTER) ||
            pInput->TriggerKey(DIK_Z))
        {
            TriggerActionByIndex(selectedIndex_);
        }
    }

    UpdateLayout();
}

void TimeUpMenu::UpdateLayout()
{
    float screenW = static_cast<float>(Window::clientWidth > 0 ? Window::clientWidth : 1600);
    float screenH = static_cast<float>(Window::clientHeight > 0 ? Window::clientHeight : 900);
    Vector2 center = { screenW * 0.5f, screenH * 0.5f };

    // 1. 全画面暗幕
    if (pOverlaySprite_)
    {
        pOverlaySprite_->SetPosition(center);
        pOverlaySprite_->SetSize({ screenW, screenH });
        pOverlaySprite_->Update();
    }

    // 2. パネル背景
    const float panelW = 500.0f;
    const float panelH = 500.0f;
    if (pPanelSprite_)
    {
        pPanelSprite_->SetPosition(center);
        pPanelSprite_->SetSize({ panelW, panelH });
        pPanelSprite_->Update();
    }

    // 3. ボタン配置
    const float baseBtnW = 341.3f;
    const float baseBtnH = 85.3f;
    const float btnSpacing = 128.0f;
    const float startY = center.y - btnSpacing; // 3項目の中心合わせ

    // カラーパレット定義 (非選択 / 選択中ハイライト)
    static const Vector4 kNormalColors[kItemCount] = {
        { 0.40f, 0.22f, 0.10f, 0.85f }, // 1. リトライ (アンバー・ウォームブラウン)
        { 0.15f, 0.25f, 0.38f, 0.85f }, // 2. ステージセレクトへ (ディープスレートブルー)
        { 0.38f, 0.15f, 0.18f, 0.85f }  // 3. タイトルへ (ディープクリムゾン)
    };

    static const Vector4 kHighlightColors[kItemCount] = {
        { 1.00f, 0.65f, 0.15f, 1.0f },  // 1. リトライ (ネオンアンバー・ゴールド)
        { 0.35f, 0.78f, 1.00f, 1.0f },  // 2. ステージセレクトへ (ネオンシアン)
        { 1.00f, 0.35f, 0.38f, 1.0f }   // 3. タイトルへ (ネオンコーラルレッド)
    };

    for (int i = 0; i < kItemCount; ++i)
    {
        if (pButtonSprites_[i])
        {
            Vector2 btnPos = { center.x, startY + static_cast<float>(i) * btnSpacing };
            pButtonSprites_[i]->SetPosition(btnPos);
			pTextSprites_[i]->SetPosition(btnPos);

            if (i == selectedIndex_)
            {
                // 選択中のボタン: 脈動ハイライト & 拡大
                float pulse = 1.0f + 0.03f * std::sin(animTimer_ * 4.0f);
                pButtonSprites_[i]->SetSize({ (baseBtnW + 40.0f) * pulse, baseBtnH * pulse });
				pTextSprites_[i]->SetSize({ baseBtnW * pulse, baseBtnH * pulse });
                pButtonSprites_[i]->SetColor(kHighlightColors[i]);

                // カーソルバーを左脇に配置
                if (pCursorSprite_)
                {
                    Vector2 cursorOffset = { -((baseBtnW + 40.0f) * 0.5f * pulse + 14.0f), 0.0f };
                    pCursorSprite_->SetPosition({ btnPos.x + cursorOffset.x, btnPos.y });
                    pCursorSprite_->SetSize({ 10.0f, baseBtnH * 0.75f * pulse });
                    pCursorSprite_->Update();
                }
            }
            else
            {
                pButtonSprites_[i]->SetSize({ (baseBtnW + 40.0f), baseBtnH });
				pTextSprites_[i]->SetSize({ baseBtnW, baseBtnH });
                pButtonSprites_[i]->SetColor(kNormalColors[i]);
            }

            pButtonSprites_[i]->Update();
			pTextSprites_[i]->Update();
        }
    }
}

void TimeUpMenu::Draw()
{
    if (!isOpen_)
    {
        return;
    }

    // 1. 暗幕オーバーレイ
    if (pOverlaySprite_)
    {
        pOverlaySprite_->Draw1F();
    }

    // 2. パネル背景
    if (pPanelSprite_)
    {
        pPanelSprite_->Draw1F();
    }

    // 3. メニューボタン
    for (int i = 0; i < kItemCount; ++i)
    {
        if (pButtonSprites_[i])
        {
            pButtonSprites_[i]->Draw1F();
        }
        if (pTextSprites_[i])
        {
            pTextSprites_[i]->Draw1F();
        }
    }

    // 4. カーソルインジケーター
    if (pCursorSprite_)
    {
        pCursorSprite_->Draw1F();
    }

    // 5. クリアテキスト
    if (pTimeUpSprite_)
    {
        float screenW = static_cast<float>(Window::clientWidth > 0 ? Window::clientWidth : 1280);
        float screenH = static_cast<float>(Window::clientHeight > 0 ? Window::clientHeight : 720);
        Vector2 center = { screenW * 0.5f, screenH * 0.5f };
        Vector2 timeUpPos = { center.x, center.y - 350.0f };
        pTimeUpSprite_->SetPosition(timeUpPos);
        pTimeUpSprite_->Update();
        pTimeUpSprite_->Draw1F();
    }

    // 6. テキストラベル補助表示 (ImGui)
    DrawOverlayUI();
}

void TimeUpMenu::DrawOverlayUI()
{
#ifdef _DEBUG
    float screenW = static_cast<float>(Window::clientWidth > 0 ? Window::clientWidth : 1600);
    float screenH = static_cast<float>(Window::clientHeight > 0 ? Window::clientHeight : 900);
    Vector2 center = { screenW * 0.5f, screenH * 0.5f };

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
                             ImGuiWindowFlags_NoBackground |
                             ImGuiWindowFlags_AlwaysAutoResize |
                             ImGuiWindowFlags_NoSavedSettings |
                             ImGuiWindowFlags_NoFocusOnAppearing |
                             ImGuiWindowFlags_NoNav |
                             ImGuiWindowFlags_NoMove;

    ImGui::SetNextWindowPos(ImVec2(center.x, center.y - 170.0f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    if (ImGui::Begin("##TimeUpHeaderUI", nullptr, flags))
    {
        if (!stageTitle_.empty())
        {
            ImGui::SetWindowFontScale(1.2f);
            ImGui::TextColored(ImVec4(0.9f, 0.8f, 0.8f, 0.9f), "%s", stageTitle_.c_str());
            ImGui::SetWindowFontScale(1.0f);
        }

        ImGui::SetWindowFontScale(1.8f);
        ImGui::TextColored(ImVec4(1.0f, 0.28f, 0.28f, 0.98f), "T I M E   U P !");
        ImGui::SetWindowFontScale(1.0f);

        ImGui::SetWindowFontScale(1.0f);
        ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.4f, 0.85f), "30 seconds deadline reached!");
        ImGui::End();
    }

    const char* kItemLabels[kItemCount] = {
        "1. Retry ( Try Again )",
        "2. Stage Select ( Coming Soon )",
        "3. Title ( Return to Title )"
    };

    const float btnSpacing = 82.0f;
    const float startY = center.y - btnSpacing;

    for (int i = 0; i < kItemCount; ++i)
    {
        ImGui::SetNextWindowPos(ImVec2(center.x, startY + static_cast<float>(i) * btnSpacing), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        std::string winName = "##TimeUpBtnText" + std::to_string(i);
        if (ImGui::Begin(winName.c_str(), nullptr, flags))
        {
            if (i == selectedIndex_)
            {
                ImGui::SetWindowFontScale(1.25f);
                if (ImGui::Selectable(("> " + std::string(kItemLabels[i]) + " <").c_str(), true))
                {
                    TriggerActionByIndex(i);
                }
                ImGui::SetWindowFontScale(1.0f);
            }
            else
            {
                ImGui::SetWindowFontScale(1.15f);
                if (ImGui::Selectable(("  " + std::string(kItemLabels[i]) + "  ").c_str(), false))
                {
                    selectedIndex_ = i;
                    TriggerActionByIndex(i);
                }
                if (ImGui::IsItemHovered())
                {
                    selectedIndex_ = i;
                }
                ImGui::SetWindowFontScale(1.0f);
            }
            ImGui::End();
        }
    }

    // 操作ガイド表示
    ImGui::SetNextWindowPos(ImVec2(center.x, center.y + 155.0f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    if (ImGui::Begin("##TimeUpFooterGuideUI", nullptr, flags))
    {
        ImGui::TextColored(ImVec4(0.85f, 0.75f, 0.75f, 0.9f),
            "[ Up / Down or W / S ]: Move    [ Space / Enter / Click ]: Select");
        ImGui::End();
    }
#endif // _DEBUG
}
