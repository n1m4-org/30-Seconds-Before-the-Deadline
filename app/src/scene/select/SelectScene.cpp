#include "SelectScene.h"
#include <Core/Window/Window.h>
#include <Core/DirectX12/TextureManager.h>
#include <config/ResourcePath.h>
#include <Features/Audio/AudioManager.h>
#include <Features/Layer/CanvasScope.h>
#include <Effects/SceneTransition/TransShutter.h>
#include <drawable/sprite/SpriteSystem.h>
#include <stage/StageManager.h>
#include <NiGui.h>
#include <dinput.h>
#include <cmath>
#include <algorithm>
#include <filesystem>
#include <format>

SelectScene::SelectScene(ISceneArgs* pArgs)
    : SceneBase(pArgs)
{
}

void SelectScene::Initialize()
{
    pInput_ = Input::GetInstance();
    pSceneManager_ = SceneManager::GetInstance();
    pDx12_ = std::any_cast<DirectX12*>(pArgs_->Get("DirectX12"));
    pCubemapSystem_ = std::any_cast<CubemapSystem*>(pArgs_->Get("CubemapSystem"));

    this->InitializeGameEye();
    this->InitializeCanvas();
    this->ScanMapFiles();
    this->BuildStageCards();

    // 元居たステージにカーソルを合わせる
    std::string lastPlayed = StageManager::GetLastPlayedStageFileName();
    selectedIndex_ = 0;
    for (size_t i = 0; i < stageCards_.size(); ++i)
    {
        if (stageCards_[i].fileName == lastPlayed)
        {
            selectedIndex_ = static_cast<int>(i);
            break;
        }
    }

    animTimer_ = 0.0f;
    currentScrollY_ = 0.0f;
    targetScrollY_ = 0.0f;
    isChangingScene_ = false;

    // 初期の目標スクロール位置を設定
    if (!stageCards_.empty() && selectedIndex_ >= 0 && selectedIndex_ < static_cast<int>(stageCards_.size()))
    {
        float cardY = stageCards_[selectedIndex_].basePos.y;
        targetScrollY_ = 480.0f - cardY;
        currentScrollY_ = targetScrollY_;
    }

    // 初期状態のスプライト座標と外観を更新
    UpdateCardVisuals();

    pBgmAudio_ = AudioManager::GetInstance()->GetNewAudio("BGM", Path::Audio::kBgmTitle);
    pBgmAudio_->SetVolume(0.075f);
    pBgmAudio_->Play(true);

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
}

void SelectScene::Finalize()
{
    pBgmAudio_->Stop();
    pLayer_->RemoveCanvas(pCanvasUI_.get());
    if (pCanvasUI_) pCanvasUI_->Finalize();

    pGameEye_.reset();
}

void SelectScene::InitializeGameEye()
{
    pGameEye_ = std::make_unique<GameEye2d>();
    pGameEye_->SetName("SelectSceneGameEye");
}

void SelectScene::InitializeCanvas()
{
    Canvas::Params params = {};
    params.name = "SelectCanvasUI";
    params.pDx12 = pDx12_;
    params.pCubemapSystem = pCubemapSystem_;
    params.pGameEye = nullptr; // スクリーンピクセル座標系
#ifdef _DEBUG
    params.pImGuiManager = std::any_cast<ImGuiManager*>(pArgs_->Get("ImGuiManager"));
#endif

    pCanvasUI_ = std::make_unique<Canvas>();
    pCanvasUI_->Initialize(params);
    pLayer_->AddCanvas(pCanvasUI_.get());
}

void SelectScene::ScanMapFiles()
{
    stageCards_.clear();
    worldList_.clear();

    std::string mapDirPath = std::string(Path::Resource::kJsonDir) + Path::Json::kMapDir;
    std::filesystem::path dir(mapDirPath);

    std::vector<std::pair<int, int>> parsedList;
    std::vector<std::string> fileList;

    if (std::filesystem::exists(dir))
    {
        for (const auto& entry : std::filesystem::directory_iterator(dir))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".json")
            {
                std::string fileName = entry.path().filename().string();
                std::string stem = entry.path().stem().string();

                int world = 0;
                int stage = 0;
                // stageW_S 形式のみ対象 (test_map 等は自然に除外)
                if (sscanf_s(stem.c_str(), "stage%d_%d", &world, &stage) == 2)
                {
                    if (world >= 1 && stage >= 1)
                    {
                        parsedList.push_back({ world, stage });
                        fileList.push_back(fileName);
                    }
                }
            }
        }
    }

    // インデックス配列を作成してソート
    std::vector<size_t> indices(parsedList.size());
    for (size_t i = 0; i < indices.size(); ++i) indices[i] = i;

    std::sort(indices.begin(), indices.end(), [&](size_t a, size_t b) {
        if (parsedList[a].first != parsedList[b].first)
            return parsedList[a].first < parsedList[b].first;
        return parsedList[a].second < parsedList[b].second;
    });

    for (size_t idx : indices)
    {
        StageCard card;
        card.world = parsedList[idx].first;
        card.stage = parsedList[idx].second;
        card.fileName = fileList[idx];
        card.displayName = std::to_string(card.world) + "-" + std::to_string(card.stage);
        stageCards_.push_back(std::move(card));

        if (std::find(worldList_.begin(), worldList_.end(), card.world) == worldList_.end())
        {
            worldList_.push_back(card.world);
        }
    }

    // 取得できなかった場合のフォールバック
    if (stageCards_.empty())
    {
        StageCard card;
        card.world = 1;
        card.stage = 1;
        card.fileName = "stage1_1.json";
        card.displayName = "1-1";
        stageCards_.push_back(std::move(card));
        worldList_.push_back(1);
    }
}

void SelectScene::BuildStageCards()
{
    TextureManager* tm = TextureManager::GetInstance();
    tm->LoadTexture(Path::Image::InGame::kTestTile); // game/tile/Simple.png
    tm->LoadTexture(Path::Image::kUINumbers);        // game/ui/number_x96y128.png

    // 全体背景スプライト (simple.png)
    pSpriteScreenBg_ = std::make_unique<Sprite>();
    pSpriteScreenBg_->Initialize(Path::Image::InGame::kTestTile);
    pSpriteScreenBg_->SetAnchorPoint({ 0.5f, 0.5f });
    pSpriteScreenBg_->SetPosition({ 800.0f, 450.0f });
    pSpriteScreenBg_->SetSize({ 1600.0f, 900.0f });
    pSpriteScreenBg_->SetColor({ 0.08f, 0.10f, 0.14f, 1.0f });

    const float cardW = 180.0f;
    const float cardH = 96.0f;
    const float gapX = 28.0f;
    const float rowSpacing = 160.0f;
    const float startY = 280.0f;

    worldHeaders_.clear();

    // ワールドごとの見出しバッジと配置計算
    for (size_t wIdx = 0; wIdx < worldList_.size(); ++wIdx)
    {
        int worldNum = worldList_[wIdx];
        float rowY = startY + static_cast<float>(wIdx) * rowSpacing;

        // ワールド見出しバッジ
        WorldHeader wh;
        wh.world = worldNum;
        wh.basePos = { 180.0f, rowY };

        wh.pBg = std::make_unique<Sprite>();
        wh.pBg->Initialize(Path::Image::InGame::kTestTile);
        wh.pBg->SetAnchorPoint({ 0.5f, 0.5f });
        wh.pBg->SetPosition(wh.basePos);
        wh.pBg->SetSize({ 140.0f, 38.0f });
        wh.pBg->SetColor({ 0.18f, 0.23f, 0.32f, 0.95f });

        wh.pDigit = std::make_unique<Sprite>();
        wh.pDigit->Initialize(Path::Image::kUINumbers);
        wh.pDigit->SetAnchorPoint({ 0.5f, 0.5f });
        wh.pDigit->SetPosition({ wh.basePos.x + 25.0f, wh.basePos.y });
        wh.pDigit->SetSize({ 20.0f, 28.0f });
        wh.pDigit->SetTextureLeftTop({ static_cast<float>(worldNum % 10) * 96.0f, 0.0f });
        wh.pDigit->SetTextureSize({ 96.0f, 128.0f });
        wh.pDigit->SetColor({ 0.85f, 0.90f, 0.98f, 1.0f });

        worldHeaders_.push_back(std::move(wh));

        // このワールドに属するステージ一覧のX座標計算
        std::vector<StageCard*> thisWorldCards;
        for (auto& card : stageCards_)
        {
            if (card.world == worldNum)
            {
                thisWorldCards.push_back(&card);
            }
        }

        int count = static_cast<int>(thisWorldCards.size());
        float rowW = static_cast<float>(count) * cardW + static_cast<float>(count - 1) * gapX;
        float startX = 850.0f - rowW * 0.5f + cardW * 0.5f;

        for (int i = 0; i < count; ++i)
        {
            float cx = startX + static_cast<float>(i) * (cardW + gapX);
            thisWorldCards[i]->basePos = { cx, rowY };
        }
    }

    // 各ステージカードのスプライト生成
    for (auto& card : stageCards_)
    {
        // 1. 選択時発光グロー (simple.png)
        card.pGlow = std::make_unique<Sprite>();
        card.pGlow->Initialize(Path::Image::InGame::kTestTile);
        card.pGlow->SetAnchorPoint({ 0.5f, 0.5f });

        // 2. 外枠 (simple.png)
        card.pBorder = std::make_unique<Sprite>();
        card.pBorder->Initialize(Path::Image::InGame::kTestTile);
        card.pBorder->SetAnchorPoint({ 0.5f, 0.5f });

        // 3. 背景 (simple.png)
        card.pBg = std::make_unique<Sprite>();
        card.pBg->Initialize(Path::Image::InGame::kTestTile);
        card.pBg->SetAnchorPoint({ 0.5f, 0.5f });

        // 4. ワールド数字 (number_x96y128.png)
        card.pWorldDigit = std::make_unique<Sprite>();
        card.pWorldDigit->Initialize(Path::Image::kUINumbers);
        card.pWorldDigit->SetAnchorPoint({ 0.5f, 0.5f });
        card.pWorldDigit->SetTextureLeftTop({ static_cast<float>(card.world % 10) * 96.0f, 0.0f });
        card.pWorldDigit->SetTextureSize({ 96.0f, 128.0f });

        // 5. ハイフン横棒 (simple.png)
        card.pHyphen = std::make_unique<Sprite>();
        card.pHyphen->Initialize(Path::Image::InGame::kTestTile);
        card.pHyphen->SetAnchorPoint({ 0.5f, 0.5f });

        // 6. ステージ数字 (number_x96y128.png)
        card.pStageDigit = std::make_unique<Sprite>();
        card.pStageDigit->Initialize(Path::Image::kUINumbers);
        card.pStageDigit->SetAnchorPoint({ 0.5f, 0.5f });
        card.pStageDigit->SetTextureLeftTop({ static_cast<float>(card.stage % 10) * 96.0f, 0.0f });
        card.pStageDigit->SetTextureSize({ 96.0f, 128.0f });

        // 7. 選択インジケーターピン (simple.png)
        card.pPin = std::make_unique<Sprite>();
        card.pPin->Initialize(Path::Image::InGame::kTestTile);
        card.pPin->SetAnchorPoint({ 0.5f, 0.5f });
    }
}

void SelectScene::Update()
{
    animTimer_ += 0.06f;

    if (pGameEye_)
    {
        pGameEye_->Update();
    }

    if (!isChangingScene_)
    {
        this->UpdateInput();
    }

    // スムーズスクロール更新
    if (!stageCards_.empty() && selectedIndex_ >= 0 && selectedIndex_ < static_cast<int>(stageCards_.size()))
    {
        float cardY = stageCards_[selectedIndex_].basePos.y;
        targetScrollY_ = 480.0f - cardY;
    }
    currentScrollY_ = std::lerp(currentScrollY_, targetScrollY_, 0.15f);

    // 全スプライトの座標・色・サイズ・GPUトランスフォーム更新
    this->UpdateCardVisuals();
}

void SelectScene::UpdateInput()
{
    if (stageCards_.empty()) return;

    int curIdx = selectedIndex_;
    int prevIdx = selectedIndex_;
    const auto& curCard = stageCards_[curIdx];

    // [D] / [→]: 次のステージへ
    if (pInput_->TriggerKey(DIK_D) || pInput_->TriggerKey(DIK_RIGHT))
    {
        selectedIndex_ = (curIdx + 1) % static_cast<int>(stageCards_.size());
    }
    // [A] / [←]: 前のステージへ
    else if (pInput_->TriggerKey(DIK_A) || pInput_->TriggerKey(DIK_LEFT))
    {
        selectedIndex_ = (curIdx - 1 + static_cast<int>(stageCards_.size())) % static_cast<int>(stageCards_.size());
    }
    // [S] / [↓]: 次のワールドのステージへ
    else if (pInput_->TriggerKey(DIK_S) || pInput_->TriggerKey(DIK_DOWN))
    {
        auto it = std::find(worldList_.begin(), worldList_.end(), curCard.world);
        if (it != worldList_.end())
        {
            size_t nextWIdx = (std::distance(worldList_.begin(), it) + 1) % worldList_.size();
            int targetWorld = worldList_[nextWIdx];

            int bestIdx = -1;
            int bestDiff = 999;
            for (size_t i = 0; i < stageCards_.size(); ++i)
            {
                if (stageCards_[i].world == targetWorld)
                {
                    int diff = std::abs(stageCards_[i].stage - curCard.stage);
                    if (diff < bestDiff)
                    {
                        bestDiff = diff;
                        bestIdx = static_cast<int>(i);
                    }
                }
            }
            if (bestIdx >= 0)
            {
                selectedIndex_ = bestIdx;
            }
        }
    }
    // [W] / [↑]: 前のワールドのステージへ
    else if (pInput_->TriggerKey(DIK_W) || pInput_->TriggerKey(DIK_UP))
    {
        auto it = std::find(worldList_.begin(), worldList_.end(), curCard.world);
        if (it != worldList_.end())
        {
            size_t prevWIdx = (std::distance(worldList_.begin(), it) - 1 + worldList_.size()) % worldList_.size();
            int targetWorld = worldList_[prevWIdx];

            int bestIdx = -1;
            int bestDiff = 999;
            for (size_t i = 0; i < stageCards_.size(); ++i)
            {
                if (stageCards_[i].world == targetWorld)
                {
                    int diff = std::abs(stageCards_[i].stage - curCard.stage);
                    if (diff < bestDiff)
                    {
                        bestDiff = diff;
                        bestIdx = static_cast<int>(i);
                    }
                }
            }
            if (bestIdx >= 0)
            {
                selectedIndex_ = bestIdx;
            }
        }
    }

    // カーソル移動SE再生
    if (selectedIndex_ != prevIdx)
    {
        if (pChoiceAudio_)
        {
            pChoiceAudio_->Play();
        }
    }

    // [Space] / [Enter]: 決定 (ゲーム開始)
    if (pInput_->TriggerKey(DIK_SPACE) || pInput_->TriggerKey(DIK_RETURN))
    {
        if (pDecisionAudio_)
        {
            pDecisionAudio_->Play();
        }
        isChangingScene_ = true;

        // 選択されたステージのファイル名をセット
        StageManager::SetSelectedStageFileName(stageCards_[selectedIndex_].fileName);
        StageManager::SetLastPlayedStageFileName(stageCards_[selectedIndex_].fileName);

        // ゲームシーンへ遷移
        pSceneManager_->ReserveScene("GameScene", std::make_unique<TransShutter>());
        return;
    }

    // [Esc]: タイトルへ戻る
    if (pInput_->TriggerKey(DIK_ESCAPE))
    {
        isChangingScene_ = true;

        // タイトルシーンへ遷移
        pSceneManager_->ReserveScene("TitleScene", std::make_unique<TransShutter>());
        return;
    }
}

void SelectScene::UpdateCardVisuals()
{
    // 1. 全体背景の更新
    if (pSpriteScreenBg_)
    {
        pSpriteScreenBg_->Update();
    }

    // 2. ワールド見出しバッジの更新
    for (auto& wh : worldHeaders_)
    {
        float cy = wh.basePos.y + currentScrollY_;
        bool visible = (cy >= 60.0f && cy <= 880.0f);

        if (wh.pBg)
        {
            wh.pBg->SetPosition({ wh.basePos.x, cy });
            wh.pBg->SetEnableDraw(visible);
            wh.pBg->Update();
        }
        if (wh.pDigit)
        {
            wh.pDigit->SetPosition({ wh.basePos.x + 25.0f, cy });
            wh.pDigit->SetEnableDraw(visible);
            wh.pDigit->Update();
        }
    }

    // 3. 各ステージカードの更新
    const float baseCardW = 180.0f;
    const float baseCardH = 96.0f;

    for (size_t i = 0; i < stageCards_.size(); ++i)
    {
        auto& card = stageCards_[i];
        bool isSelected = (static_cast<int>(i) == selectedIndex_);

        float cx = card.basePos.x;
        float cy = card.basePos.y + currentScrollY_;
        bool visible = (cy >= 50.0f && cy <= 880.0f);

        float cardW = baseCardW;
        float cardH = baseCardH;
        float scale = isSelected ? 1.08f : 1.0f;
        cardW *= scale;
        cardH *= scale;

        float pulse = 0.5f + 0.5f * std::sin(animTimer_ * 3.5f);

        // (A) 発光グロー (simple.png)
        if (card.pGlow)
        {
            card.pGlow->SetPosition({ cx, cy });
            card.pGlow->SetSize({ cardW + 12.0f, cardH + 12.0f });
            card.pGlow->SetColor({ 1.0f, 0.85f, 0.20f, (0.35f + 0.30f * pulse) });
            card.pGlow->SetEnableDraw(visible && isSelected);
            card.pGlow->Update();
        }

        // (B) 外枠 (simple.png)
        if (card.pBorder)
        {
            card.pBorder->SetPosition({ cx, cy });
            if (isSelected)
            {
                card.pBorder->SetSize({ cardW + 6.0f, cardH + 6.0f });
                card.pBorder->SetColor({ 1.0f, 0.90f, 0.30f, 1.0f }); // 鮮烈なイエローゴールド
            }
            else
            {
                card.pBorder->SetSize({ cardW + 2.0f, cardH + 2.0f });
                card.pBorder->SetColor({ 0.26f, 0.32f, 0.44f, 0.90f }); // 控えめなスレート枠線
            }
            card.pBorder->SetEnableDraw(visible);
            card.pBorder->Update();
        }

        // (C) 背景 (simple.png)
        if (card.pBg)
        {
            card.pBg->SetPosition({ cx, cy });
            card.pBg->SetSize({ cardW, cardH });
            if (isSelected)
            {
                card.pBg->SetColor({ 0.92f, 0.58f, 0.10f, 0.98f }); // 鮮やかなアンバーゴールド
            }
            else
            {
                card.pBg->SetColor({ 0.12f, 0.15f, 0.22f, 0.95f }); // 落ち着いたダークスレート
            }
            card.pBg->SetEnableDraw(visible);
            card.pBg->Update();
        }

        // (D) 中央数字 "W - S"
        const float digitW = 26.0f * scale;
        const float digitH = 36.0f * scale;
        const float hyphenW = 14.0f * scale;
        const float hyphenH = 4.0f * scale;
        const float spacing = 6.0f * scale;

        float totalW = digitW + spacing + hyphenW + spacing + digitW;
        float startX = cx - totalW * 0.5f + digitW * 0.5f;

        Vector4 numColor = isSelected ? Vector4(1.0f, 1.0f, 1.0f, 1.0f) : Vector4(0.70f, 0.76f, 0.86f, 0.95f);
        Vector4 hypColor = isSelected ? Vector4(1.0f, 0.95f, 0.85f, 1.0f) : Vector4(0.45f, 0.52f, 0.62f, 0.90f);

        if (card.pWorldDigit)
        {
            card.pWorldDigit->SetPosition({ startX, cy });
            card.pWorldDigit->SetSize({ digitW, digitH });
            card.pWorldDigit->SetColor(numColor);
            card.pWorldDigit->SetEnableDraw(visible);
            card.pWorldDigit->Update();
        }

        if (card.pHyphen)
        {
            float hyphenX = startX + digitW * 0.5f + spacing + hyphenW * 0.5f;
            card.pHyphen->SetPosition({ hyphenX, cy });
            card.pHyphen->SetSize({ hyphenW, hyphenH });
            card.pHyphen->SetColor(hypColor);
            card.pHyphen->SetEnableDraw(visible);
            card.pHyphen->Update();
        }

        if (card.pStageDigit)
        {
            float stageX = startX + digitW + spacing * 2.0f + hyphenW;
            card.pStageDigit->SetPosition({ stageX, cy });
            card.pStageDigit->SetSize({ digitW, digitH });
            card.pStageDigit->SetColor(numColor);
            card.pStageDigit->SetEnableDraw(visible);
            card.pStageDigit->Update();
        }

        // (E) 選択インジケーターピン (simple.png)
        if (card.pPin)
        {
            float pinY = cy - cardH * 0.5f - 10.0f - 3.0f * pulse;
            card.pPin->SetPosition({ cx, pinY });
            card.pPin->SetSize({ 22.0f, 6.0f });
            card.pPin->SetColor({ 1.0f, 0.90f, 0.30f, 1.0f });
            card.pPin->SetEnableDraw(visible && isSelected);
            card.pPin->Update();
        }
    }
}

void SelectScene::Draw()
{
    CanvasScope canvasScope(pCanvasUI_.get());

    // 1. 全体背景
    if (pSpriteScreenBg_)
    {
        pSpriteScreenBg_->Draw1F();
    }

    // 2. ワールド見出しバッジの描画
    for (auto& wh : worldHeaders_)
    {
        if (wh.pBg && wh.pBg->GetEnableDraw())
        {
            wh.pBg->Draw1F();
        }
        if (wh.pDigit && wh.pDigit->GetEnableDraw())
        {
            wh.pDigit->Draw1F();
        }
    }

    // 3. 各ステージカードの描画 (simple.png)
    for (auto& card : stageCards_)
    {
        // 選択時グロー
        if (card.pGlow && card.pGlow->GetEnableDraw())
        {
            card.pGlow->Draw1F();
        }
        // 外枠
        if (card.pBorder && card.pBorder->GetEnableDraw())
        {
            card.pBorder->Draw1F();
        }
        // 背景
        if (card.pBg && card.pBg->GetEnableDraw())
        {
            card.pBg->Draw1F();
        }
        // ワールド数字
        if (card.pWorldDigit && card.pWorldDigit->GetEnableDraw())
        {
            card.pWorldDigit->Draw1F();
        }
        // ハイフン
        if (card.pHyphen && card.pHyphen->GetEnableDraw())
        {
            card.pHyphen->Draw1F();
        }
        // ステージ数字
        if (card.pStageDigit && card.pStageDigit->GetEnableDraw())
        {
            card.pStageDigit->Draw1F();
        }
        // 選択ピン
        if (card.pPin && card.pPin->GetEnableDraw())
        {
            card.pPin->Draw1F();
        }
    }
}