#include "TitleScene.h"
#include <Effects/SceneTransition/TransShutter.h>
#include <drawable/sprite/SpriteSystem.h>
#include <drawable/object3d/Object3dSystem.h>
#include <drawable/line/LineSystem.h>
#include <any>
#include <Core/DirectX12/TextureManager.h>
#include <config/ResourcePath.h>
#include <Color.h>
#include <cmath>
#include <Features/Audio/AudioManager.h>
#include <Features/Layer/CanvasScope.h>
#include <Math/ViewportUnits.hpp>
#include <NiGui.h>
#include <Math/Easing.h>
#include <Xinput.h>
#include <drawable/particle/ParticleStorage.h>

void TitleScene::Initialize()
{
    /// インスタンスの取得
    pInput_ = Input::GetInstance();
    pSceneManager_ = SceneManager::GetInstance();
    pCubemapSystem_ = std::any_cast<CubemapSystem*>(pArgs_->Get("CubemapSystem"));
    pDx12_ = std::any_cast<DirectX12*>(pArgs_->Get("DirectX12"));
    pInputMapperUI_ = std::any_cast<InputMapper<InputActionUI>*>(pArgs_->Get("InputMapperUI"));
    pModelManager_ = std::any_cast<ModelManager*>(pArgs_->Get("ModelManager"));

    // ゲームアイの初期化
    this->InitializeGameEye();

    this->InitializeCanvas();

    // スカイボックスの初期化
    this->InitializeSkybox();

    // スプライトの初期化
    this->InitializeSprites();

    // パーティクルエミッタの初期化
    this->InitializeParticleEmitter();

    pPlayerPopupAnimation_ = std::make_unique<PlayerPopupAnimation>();
    pPlayerPopupAnimation_->Initialize();

	pBgmAudio_  = AudioManager::GetInstance()->GetNewAudio("BGM", Path::Audio::kBgmTitle);
    pBgmAudio_->SetVolume(0.075f);
    pBgmAudio_->Play(true);
}

void TitleScene::Finalize()
{
	pBgmAudio_->Stop();
    gameEye_.reset();
    pLayer_->RemoveCanvas(pCanvasBack_.get());
    pLayer_->RemoveCanvas(pCanvasSprite_.get());
    pLayer_->RemoveCanvas(pCanvasWorld_.get());
    pCanvasBack_->Finalize();
    pCanvasSprite_->Finalize();
    pCanvasWorld_->Finalize();
    pParticleEmitter_->Finalize();
}

void TitleScene::Update()
{
    Vector3 eyeRotate = gameEye_->GetRotation();
    eyeRotate.y += 0.001f;

    float t = (std::sinf(eyeRotate.y * 10.0f) + 1.0f) / 2.0f; // 0から1の範囲で変化する値
    Vector3 eyePos = gameEye_->GetPosition();
    eyePos.z = std::lerp(kEyePosZMin_, kEyePosZMax_, Math::Easing::EaseInOutSine(t));

    gameEye_->Update();

    if (pInputMapperUI_->IsRelease(InputActionUI::Confirm) && !isChangingScene_)
    {
        this->ChangeToGameScene();
    }

    pParticleEmitter_->Update();

    pPlayerPopupAnimation_->Update();
}

void TitleScene::Draw()
{
    CanvasScope canvasScopeWorld(pCanvasWorld_.get());
    pParticle_->Draw1F();

    CanvasScope canvasScopeUI(pCanvasSprite_.get());
    pPlayerPopupAnimation_->Draw1F();
    pTitleTextSprite_->Draw1F();
}

void TitleScene::InitializeGameEye()
{
    /// ゲームアイの初期化
    gameEye_ = std::make_unique<GameEye2d>();
    gameEye_->SetName("main");

    /// ゲームアイをセット
    Object3dSystem::GetInstance()->SetGlobalEye(gameEye_.get());
    SpriteSystem::GetInstance()->SetGlobalEye(gameEye_.get());
    LineSystem::GetInstance()->SetGlobalEye(gameEye_.get());
    ParticleSystem::GetInstance()->SetGlobalEye(gameEye_.get());
    pCubemapSystem_->SetGlobalEye(gameEye_.get());
}

void TitleScene::InitializeSprites()
{
    // クリアテキスト
    pTitleTextSprite_ = std::make_unique<Sprite>();
    pTitleTextSprite_->Initialize(Path::Image::InGame::kTitleText);
    pTitleTextSprite_->SetAnchorPoint({ 0.5f, 0.5f });
	pTitleTextSprite_->SetPosition({ 800.0f, 200.0f });
    pTitleTextSprite_->Update();
}

void TitleScene::InitializeSkybox()
{
    auto pTM = TextureManager::GetInstance();
    pTM->LoadTexture(Path::Image::kTitleSkybox);
}

void TitleScene::InitializeParticleEmitter()
{
    IModel* pModel = pModelManager_->Load(Path::Model::kParticlePlane);
    pParticle_ = ParticleStorage::GetInstance()->CreateParticle();
    pParticle_->Initialize(pModel);

    ParticleEmitter::Params params = {};
    params.particle = pParticle_;

    pParticleEmitter_ = std::make_unique<ParticleEmitter>();
    pParticleEmitter_->Initialize(params);
}

void TitleScene::InitializeCanvas()
{
    Canvas::Params params = {};
    params.name = "TitleCanvas";
    params.pDx12 = pDx12_;
    params.pCubemapSystem = pCubemapSystem_;
    params.pGameEye = gameEye_.get();
    #ifdef _DEBUG
    params.pImGuiManager = std::any_cast<ImGuiManager*>(pArgs_->Get("ImGuiManager"));
    #endif // _DEBUG

    pCanvasBack_ = std::make_unique<Canvas>();
    pCanvasBack_->Initialize(params);

    params.name = "TitleCanvasWorld";
    pCanvasWorld_ = std::make_unique<Canvas>();
    pCanvasWorld_->Initialize(params);

    params.name = "TitleCanvasUI";
    params.pGameEye = nullptr;
    pCanvasSprite_ = std::make_unique<Canvas>();
    pCanvasSprite_->Initialize(params);

    pLayer_->AddCanvas(pCanvasBack_.get());
    pLayer_->AddCanvas(pCanvasWorld_.get());
    pLayer_->AddCanvas(pCanvasSprite_.get());
}

void TitleScene::ChangeToGameScene()
{
    pTransShutter_ = std::make_unique<TransShutter>();
    pSceneManager_->ReserveScene("SelectScene", std::move(pTransShutter_));
    isChangingScene_ = true;
}
