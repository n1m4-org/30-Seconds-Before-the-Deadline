#pragma once

#include <scene/SceneBase.h>
#include <Core/Window/Window.h>
#include <Core/DirectX12/DirectX12.h>
#include <Core/DirectX12/PostEffectExecutor.h>
#include <Features/GameEye/GameEye.h>
#include <Features/Input/Input.h>
#include <Features/SceneManager/SceneManager.h>
#include <Features/Cubemap/Skybox.h>
#include <Features/Cubemap/CubemapSystem.h>
#include <Features/Layer/Canvas.h>
#include <Features/Input/InputMapper.hpp>
#include <Features/Audio/Audio.h>
#include <Effects/PostEffects/RandomFilter/RandomFilter.h>
#include <Effects/PostEffects/GaussianBloom/GaussianBloom.h>
#include <Effects/PostEffects/RadialBlur/RadialBlur.h>
#include <Effects/SceneTransition/TransShutter.h>
#include <Effects/PostEffects/Mosaic/Mosaic.h>
#include <Math/ViewportUnits.hpp>
#include <drawable/sprite/Sprite.h>
#include <presentation/animation/RadialBeat.h>
#include <logic/input/InputAction.h>
#include <memory>
#include <wrapper/InputAwareSprite.h>
#include <Features/GameEye2d/GameEye2d.h>
#include <drawable/particle/Emitter/ParticleEmitter.h>
#include <drawable/particle/Particle.h>
#include <scene/title/animation/PlayerPopupAnimation.h>
#include <Features/Animation/AnimationEditor.hpp>

/// <summary>
/// タイトルシーン
/// </summary>
class TitleScene : public SceneBase
{
public:
    TitleScene(ISceneArgs* _pArg) : SceneBase(_pArg) {};

    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize() override;

    /// <summary>
    /// 終了
    /// </summary>
    void Finalize() override;

    /// <summary>
    /// 更新
    /// </summary>
    void Update() override;

    /// <summary>
    /// 描画
    /// </summary>
    void Draw() override;


private:
    void InitializeGameEye();
    void InitializeSprites();
    void InitializeSkybox();
    void InitializeParticleEmitter();
    void InitializeCanvas();

    /// <summary>
    /// ゲームシーンに切り替える処理を行います。
    /// </summary>
    void ChangeToGameScene();
    
    static constexpr float              kEyePosZMin_                = -120.0f;      // !< カメラのZ座標の最小値
    static constexpr float              kEyePosZMax_                = 120.0f;       // !< カメラのZ座標の最大値
    static constexpr float              kBloomThresholdMin_         = 0.313f;       // !< ブルームの閾値の最小値
    static constexpr float              kPressSpaceScaleActive_     = 1.2f;         // !< スタートプロンプトのアクティブ時のスケール
    const float                         kPosYTitle_                 = Math::Viewport::Unit::vh(50.0f) - 50.0f; // !< タイトルのY座標
    bool                                isChangingScene_            = false;
    std::unique_ptr<TransShutter>       pTransShutter_              = nullptr;      // !< シャッター遷移エフェクト
    std::unique_ptr<Canvas>             pCanvasBack_                = nullptr;      // !< タイトルキャンバス
    std::unique_ptr<Canvas>             pCanvasSprite_              = nullptr;      // !< タイトルキャンバス
    std::unique_ptr<Canvas>             pCanvasWorld_               = nullptr;      // !< タイトルキャンバス
    std::unique_ptr<GameEye2d>          gameEye_                    = {};           // !< ゲームアイ
    std::unique_ptr<PlayerPopupAnimation> pPlayerPopupAnimation_    = nullptr;      // !< プレイヤーポップアップアニメーション

    std::unique_ptr<ParticleEmitter>    pParticleEmitter_           = nullptr;      // !< パーティクルエミッター
    Particle*                           pParticle_                  = nullptr;      // !< パーティクル

	Audio* pBgmAudio_ = nullptr; // !< BGMオーディオ
    std::unique_ptr<Sprite> pTitleTextSprite_ = nullptr;   // !< クリアテキスト

    /// 他クラスのインスタンス
    PostEffectExecutor*         pPostEffectExecutor_    = nullptr;      // !< ポストエフェクト実行クラス
    DirectX12*                  pDx12_                  = nullptr;      // !< DirectX12
    Input*                      pInput_                 = nullptr;      // !< 入力w
    SceneManager*               pSceneManager_          = nullptr;      // !< シーン遷移
    CubemapSystem*              pCubemapSystem_         = nullptr;      // !< キューブマップシステム
    InputMapper<InputActionUI>* pInputMapperUI_         = nullptr;      // !< 入力マッパー
    ModelManager*               pModelManager_          = nullptr;      // !< モデルマネージャー
};