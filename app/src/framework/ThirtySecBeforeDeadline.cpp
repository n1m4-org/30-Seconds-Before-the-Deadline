#include "ThirtySecBeforeDeadline.h"

#include <Common/define.h>
#include <Features/SceneManager/SceneManager.h>
#include <scene/factory/SceneFactory.h>
#include <Features/Model/Helper/ModelHelper.h>
#include <NiGui.h>
#include <config/ResourcePath.h>
#include <Features/DeltaTimeManager/DeltaTimeManager.h>
#include <algorithm>

#pragma comment(lib, "Ws2_32.lib")

void ThirtySecBeforeDeadline::Initialize()
{
    /// 基底クラスの初期化処理
    NimaFramework::Initialize();

    #ifdef _DEBUG
    pImGuiManager_->EnableDocking();
    #endif
    
    /// シーンファクトリの設定
    pSceneFactory_ = std::make_unique<SceneFactory>();
    pSceneManager_->SetSceneFactory(pSceneFactory_.get());
    pIntermediateScreenFactory_ = std::make_unique<IntermediateScreenFactory>();
    pTransitionExecutor_->SetIntermediateScreenFactory(pIntermediateScreenFactory_.get());

    /// 当たり判定マネージャの初期化
    pCollisionManager_ = CollisionManager::GetInstance();
    pCollisionManager_->Initialize();

    /// モデルローダー、ストレージ、マネージャの初期化
    pModelLoader_ = Helper::Model::CreateLoader<ModelLoaderAssimp>(pDirectX_.get());
    pModelStorage_ = Helper::Model::CreateStorage();
    pModelManager_ = Helper::Model::CreateManager(pModelLoader_.get(), pModelStorage_.get());

    // モデルマネージャをシーンマネージャにセット
    pSceneManager_->SetModelManager(pModelManager_.get());

    /// 入力マッパーの初期化
    this->InitializeInputMapper();

    /// Dissolve用のテクスチャをロード
    pTextureManager_->LoadTexture("noise0.png");

    /// デルタタイムマネージャの初期化
    DeltaTimeManager::GetInstance()->SetDeltaTime(DeltaTimeChannelReserved::Particle, 1.0f / 60.0f);
}

void ThirtySecBeforeDeadline::Finalize()
{
    /// 基底クラスの終了処理
    NimaFramework::Finalize();
}

void ThirtySecBeforeDeadline::Update()
{
    // フレーム間の経過時間 (DeltaTime) を計測
    auto now = std::chrono::steady_clock::now();
    float deltaTime = 1.0f / 60.0f;

    if (!isFirstUpdate_)
    {
        std::chrono::duration<float> elapsed = now - lastUpdateTime_;
        deltaTime = elapsed.count();
        // 極端な処理落ち・デバッグ停止・ウィンドウドラッグ等による異常値を保護
        deltaTime = std::clamp(deltaTime, 0.0001f, 0.1f);
    }
    else
    {
        isFirstUpdate_ = false;
    }
    lastUpdateTime_ = now;

    // デルタタイムマネージャへ最新値を登録
    auto dtManager = DeltaTimeManager::GetInstance();
    dtManager->SetDeltaTime(DeltaTimeChannelReserved::Default, deltaTime);
    dtManager->SetDeltaTime(DeltaTimeChannelReserved::Game, deltaTime);
    dtManager->SetDeltaTime(DeltaTimeChannelReserved::Particle, deltaTime);

    /// 当たり判定の更新
    pCollisionManager_->CheckAllCollision();

    /// 基底クラスの更新処理
    NimaFramework::Update();
}

void ThirtySecBeforeDeadline::Draw()
{
    /// 描画前処理
    NimaFramework::PreProcess();


    /// バックバッファ書き込み
    NimaFramework::Draw();


    /// 描画後処理
    NimaFramework::PostProcess();
}

void ThirtySecBeforeDeadline::InitializeInputMapper()
{
    pInputMapperUI_ = std::make_unique<InputMapper<InputActionUI>>();
    pInputMapperUI_->MapAction(InputActionUI::Confirm, { InputMapper<InputActionUI>::KeyCode{ DIK_SPACE } });
    pInputMapperUI_->MapAction(InputActionUI::Confirm, { InputMapper<InputActionUI>::PadButton{ XINPUT_GAMEPAD_A } });
    pInputMapperUI_->MapAction(InputActionUI::Cancel, { InputMapper<InputActionUI>::KeyCode{ DIK_ESCAPE } });
    pInputMapperUI_->MapAction(InputActionUI::Cancel, { InputMapper<InputActionUI>::PadButton{ XINPUT_GAMEPAD_B } });
    pInputMapperUI_->MapAction(InputActionUI::Pause, { InputMapper<InputActionUI>::KeyCode{ DIK_ESCAPE } });
    pInputMapperUI_->MapAction(InputActionUI::Pause, { InputMapper<InputActionUI>::PadButton{ XINPUT_GAMEPAD_START } });

    pSceneManager_->AddInitialArg("InputMapperUI", pInputMapperUI_.get());
}
