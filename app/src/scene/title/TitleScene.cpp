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

    /// フィルタの初期化と登録
    {
        auto tempBloom = pCanvasBack_->GetPostEffectExecutor().AddEffect(PostEffectClassName::GaussianBloom);
        auto tempGaussian = pCanvasBack_->GetPostEffectExecutor().AddEffect(PostEffectClassName::SeparatedGaussianFilter);
        auto tempMosaic = pCanvasBack_->GetPostEffectExecutor().AddEffect(PostEffectClassName::Mosaic);
        auto tempRadial = pCanvasBack_->GetPostEffectExecutor().AddEffect(PostEffectClassName::RadialBlur);
        tempRadial->Enable(true);
        pGaussianBloom_ = static_cast<GaussianBloom*>(tempBloom);
        pMosaic_ = static_cast<Mosaic*>(tempMosaic);
        pSeparatedGaussianFilter_ = static_cast<SeparatedGaussianFilter*>(tempGaussian);
    }

    this->InitializePostEffects();

    pSoundStartButton_ = AudioManager::GetInstance()->GetNewAudio("Effect", Path::Audio::kSeStartButton);
    pSoundStartButton_->SetVolume(0.1f);

    pSoundBGM_ = AudioManager::GetInstance()->GetNewAudio("BGM", Path::Audio::kBgmTitle);
    pSoundBGM_->SetVolume(0.075f);
    pSoundBGM_->Play(true);

    pPlayerPopupAnimation_ = std::make_unique<PlayerPopupAnimation>();
    pPlayerPopupAnimation_->Initialize();

    // パーティクルエミッタの初期化
    this->InitializeParticleEmitter();
}

void TitleScene::Finalize()
{
    pSoundBGM_->Stop();
    gameEye_.reset();
    pLayer_->RemoveCanvas(pCanvasBack_.get());
    pLayer_->RemoveCanvas(pCanvasSprite_.get());
    pCanvasBack_->Finalize();
    pCanvasSprite_->Finalize();
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

    float threshold = std::lerp(kBloomThresholdMin_, 0.5f, Math::Easing::EaseInOutSine(t));
    pGaussianBloom_->SetThreshold(threshold);

    t = (std::sinf(eyeRotate.y * 20.0f) + 1.0f) / 2.0f; // 0から1の範囲で変化する値
    float kernelSize = std::lerp(3.0f, 31.0f, Math::Easing::EaseInOutQuad(t));
    pSeparatedGaussianFilter_->GetOption().kernelSize = static_cast<int>(kernelSize);
    pSeparatedGaussianFilter_->CreateKernel();

    
    if (pInputMapperUI_->IsRelease(InputActionUI::Confirm) && !isChangingScene_)
    {
        this->ChangeToGameScene();
    }

    if (isChangingScene_)
    {
        pSoundBGM_->SetVolume(pSoundBGM_->GetVolume() * 0.95f);
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
    TextureManager* tm = TextureManager::GetInstance();
    tm->LoadTexture(Path::Image::kTitleStartPromptSpaceKey);
    tm->LoadTexture(Path::Image::kTitleStartPromptButtonA);
}

void TitleScene::InitializeSkybox()
{
    auto pTM = TextureManager::GetInstance();
    pTM->LoadTexture(Path::Image::kTitleSkybox);
}

void TitleScene::InitializePostEffects()
{
    pGaussianBloom_->Enable(true);
    pSeparatedGaussianFilter_->Enable(true);
    pMosaic_->Enable(true);

    pGaussianBloom_->SetKernelSize(31);
    pGaussianBloom_->SetSigma(27.9f);
    pGaussianBloom_->SetThreshold(0.313f);
    pGaussianBloom_->SetBloomIntensity(2.14f);

    pSeparatedGaussianFilter_->SetSigma(27.0f);
    pMosaic_->GetOption().power = 200.0f;
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
    pSoundStartButton_->Play();
    pTransShutter_ = std::make_unique<TransShutter>();
    pSceneManager_->ReserveScene("GameScene", std::move(pTransShutter_));
    isChangingScene_ = true;
}
