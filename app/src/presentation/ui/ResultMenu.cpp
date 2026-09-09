#include "ResultMenu.h"
#include <Core/Window/Window.h>
#include <Core/DirectX12/TextureManager.h>
#include <config/ResourcePath.h>
#include <NiGui.h>
#include <dinput.h>
#include <cmath>
#include <algorithm>
#include <string>

ResultMenu::ResultMenu()
{
}

ResultMenu::~ResultMenu()
{
}

void ResultMenu::Initialize()
{
    TextureManager* tm = TextureManager::GetInstance();
    tm->LoadTexture(Path::Image::InGame::kTestTile); // game/tile/Simple.png をロード

    // 1. 全画面半透明オーバーレイ
    pOverlaySprite_ = std::make_unique<Sprite>();
    pOverlaySprite_->Initialize(Path::Image::InGame::kTestTile);
    pOverlaySprite_->SetAnchorPoint({ 0.5f, 0.5f });
    pOverlaySprite_->SetColor({ 0.0f, 0.0f, 0.0f, 0.68f });

    // 2. メニューパネル背景
    pPanelSprite_ = std::make_unique<Sprite>();
    pPanelSprite_->Initialize(Path::Image::InGame::kTestTile);
    pPanelSprite_->SetAnchorPoint({ 0.5f, 0.5f });
    pPanelSprite_->SetColor({ 0.09f, 0.14f, 0.17f, 0.94f });

    // 3. メニューボタン (Simple.png による代用仮配置)
    for (int i = 0; i < kItemCount; ++i)
    {
        pButtonSprites_[i] = std::make_unique<Sprite>();
        pButtonSprites_[i]->Initialize(Path::Image::InGame::kTestTile);
        pButtonSprites_[i]->SetAnchorPoint({ 0.5f, 0.5f });
    }
    pTextSprites_[0] = std::make_unique<Sprite>();
    pTextSprites_[0]->Initialize(Path::Image::InGame::kNextStageText);
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
    pCursorSprite_->SetColor({ 1.0f, 1.0f, 1.0f, 0.95f });

    // 5. クリアテキスト
    pClearSprite_ = std::make_unique<Sprite>();
    pClearSprite_->Initialize(Path::Image::InGame::kClearText);
    pClearSprite_->SetAnchorPoint({ 0.5f, 0.5f });
    pClearSprite_->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });

    selectedIndex_ = kNextStage;
    isOpen_ = false;
    currentAction_ = ResultMenuAction::None;
    animTimer_ = 0.0f;

    UpdateLayout();
}

void ResultMenu::Open()
{
    isOpen_ = true;
    selectedIndex_ = kNextStage; // 開いた時は一番上の「次のステージへ」を選択
    currentAction_ = ResultMenuAction::None;
    animTimer_ = 0.0f;
    UpdateLayout();
}

void ResultMenu::Close()
{
    isOpen_ = false;
    currentAction_ = ResultMenuAction::None;
}

ResultMenuAction ResultMenu::ConsumeAction()
{
    ResultMenuAction action = currentAction_;
    currentAction_ = ResultMenuAction::None;
    return action;
}

void ResultMenu::TriggerActionByIndex(int index)
{
    switch (index)
    {
    case kNextStage:
        currentAction_ = hasNextStage_ ? ResultMenuAction::NextStage : ResultMenuAction::Title;
        break;
    case kStageSelect:
        currentAction_ = ResultMenuAction::StageSelect;
        break;
    case kTitle:
        currentAction_ = ResultMenuAction::Title;
        break;
    default:
        break;
    }
}

void ResultMenu::Update(Input* pInput)
{
    if (!isOpen_)
    {
        return;
    }

    animTimer_ += 0.05f;

    if (pInput)
    {
        // 1. キーボード移動 (↑ 矢印キー または W キー)
        if (pInput->TriggerKey(DIK_UP) || pInput->TriggerKey(DIK_W))
        {
            selectedIndex_ = (selectedIndex_ + kItemCount - 1) % kItemCount;
        }
        // 下移動 (↓ 矢印キー または S キー)
        else if (pInput->TriggerKey(DIK_DOWN) || pInput->TriggerKey(DIK_S))
        {
            selectedIndex_ = (selectedIndex_ + 1) % kItemCount;
        }

        // 2. マウスホバー & クリック判定
        /*float screenW = static_cast<float>(Window::clientWidth > 0 ? Window::clientWidth : 1280);
        float screenH = static_cast<float>(Window::clientHeight > 0 ? Window::clientHeight : 720);
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

void ResultMenu::UpdateLayout()
{
    float screenW = static_cast<float>(Window::clientWidth > 0 ? Window::clientWidth : 1280);
    float screenH = static_cast<float>(Window::clientHeight > 0 ? Window::clientHeight : 720);
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
    const float startY = center.y - btnSpacing; // 3項目の中心合わせ (-82, 0, +82)

    // カラーパレット定義 (非選択 / 選択中ハイライト)
    static const Vector4 kNormalColors[kItemCount] = {
        { 0.18f, 0.44f, 0.28f, 0.85f }, // 1. 次のステージへ (グリーン系)
        { 0.38f, 0.34f, 0.18f, 0.85f }, // 2. ステージセレクトへ (ゴールド系)
        { 0.44f, 0.20f, 0.20f, 0.85f }  // 3. タイトルへ (赤系)
    };

    static const Vector4 kHighlightColors[kItemCount] = {
        { 0.25f, 0.95f, 0.45f, 1.0f },  // 1. 次のステージへ (発光エメラルドグリーン)
        { 1.00f, 0.84f, 0.25f, 1.0f },  // 2. ステージセレクトへ (発光イエローゴールド)
        { 1.00f, 0.36f, 0.32f, 1.0f }   // 3. タイトルへ (発光コーラルレッド)
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

void ResultMenu::Draw()
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
	if (pClearSprite_)
	{
		float screenW = static_cast<float>(Window::clientWidth > 0 ? Window::clientWidth : 1280);
		float screenH = static_cast<float>(Window::clientHeight > 0 ? Window::clientHeight : 720);
		Vector2 center = { screenW * 0.5f, screenH * 0.5f };
		Vector2 clearPos = { center.x, center.y - 350.0f };
		pClearSprite_->SetPosition(clearPos);
		pClearSprite_->Update();
		pClearSprite_->Draw1F();
	}

    // 6. テキストラベル補助表示 (ImGui)
    DrawOverlayUI();
}

void ResultMenu::DrawOverlayUI()
{
#ifdef _DEBUG
    float screenW = static_cast<float>(Window::clientWidth > 0 ? Window::clientWidth : 1280);
    float screenH = static_cast<float>(Window::clientHeight > 0 ? Window::clientHeight : 720);
    Vector2 center = { screenW * 0.5f, screenH * 0.5f };

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
                             ImGuiWindowFlags_NoBackground |
                             ImGuiWindowFlags_AlwaysAutoResize |
                             ImGuiWindowFlags_NoSavedSettings |
                             ImGuiWindowFlags_NoFocusOnAppearing |
                             ImGuiWindowFlags_NoNav |
                             ImGuiWindowFlags_NoMove;

    ImGui::SetNextWindowPos(ImVec2(center.x, center.y - 170.0f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    if (ImGui::Begin("##ResultHeaderUI", nullptr, flags))
    {
        if (!stageTitle_.empty())
        {
            ImGui::SetWindowFontScale(1.2f);
            ImGui::TextColored(ImVec4(0.8f, 0.9f, 1.0f, 0.9f), "%s", stageTitle_.c_str());
            ImGui::SetWindowFontScale(1.0f);
        }

        ImGui::SetWindowFontScale(1.8f);
        if (hasNextStage_)
        {
            ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.5f, 0.95f), "S T A G E   C L E A R !");
        }
        else
        {
            ImGui::TextColored(ImVec4(1.0f, 0.84f, 0.25f, 0.95f), "A L L   S T A G E S   C L E A R E D !");
        }
        ImGui::SetWindowFontScale(1.0f);
        ImGui::End();
    }

    std::string nextStageLabel = hasNextStage_ ? "1. Next Stage" : "1. Return to Title (Game Complete!)";
    const char* kItemLabels[kItemCount] = {
        nextStageLabel.c_str(),
        "2. Stage Select ( Coming Soon )",
        "3. Title ( Return to Title )"
    };

    const float btnSpacing = 82.0f;
    const float startY = center.y - btnSpacing;

    for (int i = 0; i < kItemCount; ++i)
    {
        ImGui::SetNextWindowPos(ImVec2(center.x, startY + static_cast<float>(i) * btnSpacing), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        std::string winName = "##ResultBtnText" + std::to_string(i);
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
    if (ImGui::Begin("##ResultFooterGuideUI", nullptr, flags))
    {
        ImGui::TextColored(ImVec4(0.7f, 0.85f, 0.75f, 0.9f),
            "[ Up / Down or W / S ]: Move    [ Space / Enter / Click ]: Select");
        ImGui::End();
    }
#endif // _DEBUG
}
