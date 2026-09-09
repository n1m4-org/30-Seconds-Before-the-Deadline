#include "GameScene.h"
#include <drawable/sprite/SpriteSystem.h>
#include <drawable/object3d/Object3dSystem.h>
#include <drawable/line/LineSystem.h>
#include <Core/DirectX12/TextureManager.h>
#include <config/ResourcePath.h>
#include <Features/Layer/CanvasScope.h>
#include <Features/Audio/AudioManager.h>
#include <Effects/SceneTransition/TransShutter.h>
#include <Effects/PostEffects/Scanline/Scanline.h>
#include <NiGui.h>
#include <dinput.h>
#include <any>

void GameScene::Initialize()
{
    /// インスタンスの取得
    pInput_ = Input::GetInstance();
    pSceneManager_ = SceneManager::GetInstance();
    pCubemapSystem_ = std::any_cast<CubemapSystem*>(pArgs_->Get("CubemapSystem"));
    pDx12_ = std::any_cast<DirectX12*>(pArgs_->Get("DirectX12"));
    pInputMapperUI_ = std::any_cast<InputMapper<InputActionUI>*>(pArgs_->Get("InputMapperUI"));
	pInGameUI_ = std::make_unique<InGameUI>();
    pInGameUI_->Initialize();


    /// Canvasの初期化
    {
        Canvas::Params params = {};
        params.name = "GameCanvas";
        params.pDx12 = pDx12_;
        params.pCubemapSystem = pCubemapSystem_;
#ifdef _DEBUG
        params.pImGuiManager = std::any_cast<ImGuiManager*>(pArgs_->Get("ImGuiManager"));
#endif // _DEBUG

        pCanvasBack_ = std::make_unique<Canvas>();
        pCanvasBack_->Initialize(params);
        IPostEffect* scanline = pCanvasBack_->GetPostEffectExecutor().AddEffect(
            PostEffectClassName::Scanline
        );
        scanline->Enable(true);
        auto* concrete = static_cast<Scanline*>(scanline);
        Scanline::ScanlineOption* options = &concrete->GetOption();
		options->color0 = { 0.38f,0.38f,0.38f,1.0f };
		options->color1 = { 0.29f,0.29f,0.29f,1.0f };
        options->division = 25;
        options->speed = 0.28f;

        params.name = "GameCanvasSprite";
        pCanvasSprite_ = std::make_unique<Canvas>();
        pCanvasSprite_->Initialize(params);

		params.name = "GameCanvasUI";
		pCanvasUI_ = std::make_unique<Canvas>();
		pCanvasUI_->Initialize(params);

        pLayer_->AddCanvas(pCanvasBack_.get());
        pLayer_->AddCanvas(pCanvasSprite_.get());
        pLayer_->AddCanvas(pCanvasUI_.get());
    }

    // カメラの初期化
    this->InitializeGameEye();

    // スカイボックスの初期化
    this->InitializeSkybox();

    // ステージ管理クラスの初期化
    pStageManager_ = std::make_unique<StageManager>();
    pStageManager_->Initialize();

    // チュートリアルテキスト表示判定 (ステージ1-1のみ表示)
    {
        int w = 0, s = 0;
        bool isStage1_1 = (pStageManager_->GetCurrentLoadedMapFile() == "stage1_1.json") ||
                          (pStageManager_->ParseStageFileName(pStageManager_->GetCurrentLoadedMapFile(), w, s) && w == 1 && s == 1);
        pInGameUI_->SetIsStage1_1(isStage1_1);
    }

    // ポーズメニューの初期化
    pPauseMenu_ = std::make_unique<PauseMenu>();
    pPauseMenu_->Initialize();

    // リザルトメニューの初期化
    pResultMenu_ = std::make_unique<ResultMenu>();
    pResultMenu_->Initialize();

    // タイムアップメニューの初期化
    pTimeUpMenu_ = std::make_unique<TimeUpMenu>();
    pTimeUpMenu_->Initialize();

    isPaused_ = false;
    isResult_ = false;
    isTimeUp_ = false;
    isChangingScene_ = false;

    pBgmAudio_ = AudioManager::GetInstance()->GetNewAudio("BGM", Path::Audio::kBgmInGame);
    pBgmAudio_->SetVolume(0.075f);
    pBgmAudio_->Play(true);
}

void GameScene::Finalize()
{
    pBgmAudio_->Stop();
    pTimeUpMenu_.reset();
    pResultMenu_.reset();
    pPauseMenu_.reset();
    pStageManager_.reset();
    pSkybox_.reset();

    gameEye_.reset();
    pLayer_->RemoveCanvas(pCanvasBack_.get());
    pLayer_->RemoveCanvas(pCanvasSprite_.get());
	pLayer_->RemoveCanvas(pCanvasUI_.get());
    pCanvasBack_->Finalize();
    pCanvasSprite_->Finalize();
    pCanvasUI_->Finalize();
}

void GameScene::Update()
{
    // シーン遷移中は更新をスキップ
    if (isChangingScene_)
    {
        return;
    }

    // Escキーによるポーズメニューの開閉トグル (リザルト中・タイムアップ中以外)
    if (pInput_ && !isResult_ && !isTimeUp_)
    {
        if (pInput_->TriggerKey(DIK_ESCAPE))
        {
            isPaused_ = !isPaused_;
            if (isPaused_)
            {
                pPauseMenu_->Open();
            }
            else
            {
                pPauseMenu_->Close();
            }
        }
    }

    // エディットモード時はリザルトやタイムアップ画面を自動で閉じる
    if (pStageManager_ && pStageManager_->IsEditMode())
    {
        if (isResult_)
        {
            isResult_ = false;
            if (pResultMenu_) pResultMenu_->Close();
        }
        if (isTimeUp_)
        {
            isTimeUp_ = false;
            if (pTimeUpMenu_) pTimeUpMenu_->Close();
        }
    }

    // 1. ポーズ中の更新処理
    if (isPaused_)
    {
        if (pPauseMenu_)
        {
            pPauseMenu_->Update(pInput_);

            PauseMenuAction action = pPauseMenu_->ConsumeAction();
            if (action == PauseMenuAction::Resume)
            {
                // ゲームへ戻る
                isPaused_ = false;
                pPauseMenu_->Close();
            }
            else if (action == PauseMenuAction::StageSelect)
            {
                // ステージセレクトへ遷移
                isChangingScene_ = true;
                isPaused_ = false;
                pPauseMenu_->Close();
                pSceneManager_->ReserveScene("SelectScene", std::make_unique<TransShutter>());
                return;
            }
            else if (action == PauseMenuAction::Title)
            {
                // タイトルへ (シャッタートランジション付きで遷移)
                isChangingScene_ = true;
                isPaused_ = false;
                pPauseMenu_->Close();
                pSceneManager_->ReserveScene("TitleScene", std::make_unique<TransShutter>());
                return;
            }
        }
    }
    // 2. リザルト中の更新処理
    else if (isResult_)
    {
        if (pResultMenu_)
        {
            pResultMenu_->Update(pInput_);

            ResultMenuAction action = pResultMenu_->ConsumeAction();
            if (action == ResultMenuAction::NextStage)
            {
                // 次のステージへ進む (リザルトを閉じる)
                isResult_ = false;
                pResultMenu_->Close();
                if (pStageManager_)
                {
                    if (pStageManager_->HasNextStage())
                    {
                        pStageManager_->LoadNextStage();
                    }
                    else
                    {
                        // 次のステージが存在しない (全ステージクリア) 場合はタイトルへ遷移
                        isChangingScene_ = true;
                        pSceneManager_->ReserveScene("TitleScene", std::make_unique<TransShutter>());
                        return;
                    }
                }
            }
            else if (action == ResultMenuAction::StageSelect)
            {
                // ステージセレクトへ遷移
                isChangingScene_ = true;
                isResult_ = false;
                pResultMenu_->Close();
                pSceneManager_->ReserveScene("SelectScene", std::make_unique<TransShutter>());
                return;
            }
            else if (action == ResultMenuAction::Title)
            {
                // タイトルへ (シャッタートランジション付きで遷移)
                isChangingScene_ = true;
                isResult_ = false;
                pResultMenu_->Close();
                pSceneManager_->ReserveScene("TitleScene", std::make_unique<TransShutter>());
                return;
            }
        }
    }
    // 3. タイムアップ (時間切れ) 中の更新処理
    else if (isTimeUp_)
    {
        if (pTimeUpMenu_)
        {
            pTimeUpMenu_->Update(pInput_);

            TimeUpMenuAction action = pTimeUpMenu_->ConsumeAction();
            if (action == TimeUpMenuAction::Retry)
            {
                // リトライ: ステージを初期状態にリセットしてゲーム再開
                isTimeUp_ = false;
                pTimeUpMenu_->Close();
                if (pStageManager_)
                {
                    pStageManager_->ResetStage();
                }
            }
            else if (action == TimeUpMenuAction::StageSelect)
            {
                // ステージセレクトへ遷移
                isChangingScene_ = true;
                isTimeUp_ = false;
                pTimeUpMenu_->Close();
                pSceneManager_->ReserveScene("SelectScene", std::make_unique<TransShutter>());
                return;
            }
            else if (action == TimeUpMenuAction::Title)
            {
                // タイトルへ (シャッタートランジション付きで遷移)
                isChangingScene_ = true;
                isTimeUp_ = false;
                pTimeUpMenu_->Close();
                pSceneManager_->ReserveScene("TitleScene", std::make_unique<TransShutter>());
                return;
            }
        }
    }
    // 4. 通常プレイ時 / エディットモード時の更新処理 (ステージ更新)
    else
    {
        if (pStageManager_)
        {
            pStageManager_->Update(pInput_);
            int w = 0, s = 0;
            bool isStage1_1 = (pStageManager_->GetCurrentLoadedMapFile() == "stage1_1.json") ||
                              (pStageManager_->ParseStageFileName(pStageManager_->GetCurrentLoadedMapFile(), w, s) && w == 1 && s == 1);
            pInGameUI_->Update(pStageManager_->GetPcDataProgress(), pStageManager_->GetRemainingTime(), isStage1_1);

            // ステージクリア時の処理 (Playモード時のみ)
            if (pStageManager_->IsCleared())
            {
                isResult_ = true;
                if (pResultMenu_)
                {
                    pResultMenu_->SetHasNextStage(pStageManager_->HasNextStage());
                    pResultMenu_->SetStageTitle(pStageManager_->GetCurrentStageDisplayName() + " CLEARED!");
                    pResultMenu_->Open();
                }
            }
            // 制限時間切れ (タイムアップ) 時の処理 (Playモード時のみ)
            else if (pStageManager_->IsTimeUp())
            {
                isTimeUp_ = true;
                if (pTimeUpMenu_)
                {
                    pTimeUpMenu_->SetStageTitle(pStageManager_->GetCurrentStageDisplayName());
                    pTimeUpMenu_->Open();
                }
            }
        }
    }
}

void GameScene::Draw()
{
    CanvasScope canvasScopeBack(pCanvasSprite_.get());

    // 1. ステージの描画 (床・壁タイル、電波、配置オブジェクト)
    if (pStageManager_)
    {
        pStageManager_->Draw();
    }

    CanvasScope canvasScopeUI(pCanvasUI_.get());

    // 2. インゲームUIの描画
    if (pInGameUI_)
    {
        pInGameUI_->Draw();
    }

    // 3. タイムアップメニューの描画 (最前面オーバーレイ)
    if (isTimeUp_ && pTimeUpMenu_)
    {
        pTimeUpMenu_->Draw();
    }

    // 4. リザルトメニューの描画 (最前面オーバーレイ)
    if (isResult_ && pResultMenu_)
    {
        pResultMenu_->Draw();
    }

    // 5. ポーズメニューの描画 (最前面オーバーレイ)
    if (isPaused_ && pPauseMenu_)
    {
        pPauseMenu_->Draw();
    }
}

void GameScene::InitializeGameEye()
{
    /// ゲームアイの初期化
    gameEye_ = std::make_unique<GameEye>();
    gameEye_->SetName("gameEye");
    gameEye_->SetTranslate(Vector3(0, 15.0f, -30.0f));
    gameEye_->SetRotate(Vector3(-1.2f, 0, 0));
    gameEye_->SetFov(1.2f);

    /// ゲームアイをセット
    Object3dSystem::GetInstance()->SetGlobalEye(gameEye_.get());
    SpriteSystem::GetInstance()->SetGlobalEye(gameEye_.get());
    LineSystem::GetInstance()->SetGlobalEye(gameEye_.get());
    pCubemapSystem_->SetGlobalEye(gameEye_.get());
}

void GameScene::InitializeSkybox()
{
    auto pTM = TextureManager::GetInstance();
    pTM->LoadTexture(Path::Image::kTitleSkybox);

    pSkybox_ = std::make_unique<Skybox>();
    pSkybox_->Initialize(pCubemapSystem_);
    pSkybox_->SetSkyboxTexture(pTM->GetSrvHandleGPU(Path::Image::kTitleSkybox));

    pCanvasBack_->RegisterDrawable(pSkybox_.get());
}
