#include "GameScene.h"
#include <drawable/sprite/SpriteSystem.h>
#include <drawable/object3d/Object3dSystem.h>
#include <drawable/line/LineSystem.h>
#include <Core/DirectX12/TextureManager.h>
#include <config/ResourcePath.h>
#include <Features/Layer/CanvasScope.h>
#include <Effects/SceneTransition/TransShutter.h>
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

        params.name = "GameCanvas2";
        pCanvasSprite_ = std::make_unique<Canvas>();
        pCanvasSprite_->Initialize(params);

        pLayer_->AddCanvas(pCanvasBack_.get());
        pLayer_->AddCanvas(pCanvasSprite_.get());
    }

    // カメラの初期化
    this->InitializeGameEye();

    // スカイボックスの初期化
    this->InitializeSkybox();

    // ステージ管理クラスの初期化
    pStageManager_ = std::make_unique<StageManager>();
    pStageManager_->Initialize();

    // ポーズメニューの初期化
    pPauseMenu_ = std::make_unique<PauseMenu>();
    pPauseMenu_->Initialize();
    isPaused_ = false;
    isChangingScene_ = false;
}

void GameScene::Finalize()
{
    pPauseMenu_.reset();
    pStageManager_.reset();
    pSkybox_.reset();

    gameEye_.reset();
    pLayer_->RemoveCanvas(pCanvasBack_.get());
    pLayer_->RemoveCanvas(pCanvasSprite_.get());
    pCanvasBack_->Finalize();
    pCanvasSprite_->Finalize();
}

void GameScene::Update()
{
    // シーン遷移中は更新をスキップ
    if (isChangingScene_)
    {
        return;
    }

    // Escキーによるポーズメニューの開閉トグル
    if (pInput_ && !ImGui::GetIO().WantCaptureKeyboard)
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

    // ポーズ中の更新処理
    if (isPaused_)
    {
        if (pPauseMenu_)
        {
            pPauseMenu_->Update(pInput_);

            PauseMenuAction action = pPauseMenu_->ConsumeAction();
            if (action == PauseMenuAction::Resume)
            {
                // 1. ゲームへ戻る
                isPaused_ = false;
                pPauseMenu_->Close();
            }
            else if (action == PauseMenuAction::StageSelect)
            {
                // 2. ステージセレクトへ (現在は未実装の仮配置のため通知/待機)
                // 将来的に StageSelectScene への遷移を実装
            }
            else if (action == PauseMenuAction::Title)
            {
                // 3. タイトルへ (シャッタートランジション付きで遷移)
                isChangingScene_ = true;
                isPaused_ = false;
                pPauseMenu_->Close();
                pSceneManager_->ReserveScene("TitleScene", std::make_unique<TransShutter>());
                return;
            }
        }
    }
    // 通常プレイ時の更新処理 (ステージ更新)
    else
    {
        if (pStageManager_)
        {
            pStageManager_->Update(pInput_);
            pInGameUI_->Update(pStageManager_->GetPcDataProgress());

            // ステージクリア時の処理
            if (pStageManager_->IsCleared())
            {
                // 現状はPC自身が点滅演出を行い、エディタ上で接続完了が表示される
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

    // 2. インゲームUIの描画
    if(pInGameUI_)
    {
		pInGameUI_->Draw();
    }

    // 3. ポーズメニューの描画 (最前面オーバーレイ)
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
