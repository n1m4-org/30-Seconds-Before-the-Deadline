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
#include <logic/input/InputAction.h>
#include <stage/StageManager.h>
#include <presentation/ui/PauseMenu.h>
#include <presentation/ui/InGameUI.h>
#include <memory>

/// <summary>
/// ゲームプレイシーン
/// </summary>
class GameScene : public SceneBase
{
public:
    GameScene(ISceneArgs* _pArg) : SceneBase(_pArg) {};

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
    void InitializeSkybox();

private:
    std::unique_ptr<Canvas>     pCanvasBack_ = nullptr;      // !< 背景キャンバス
    std::unique_ptr<Canvas>     pCanvasSprite_ = nullptr;    // !< スプライトキャンバス
    std::unique_ptr<GameEye>    gameEye_ = {};               // !< ゲームカメラ
    std::unique_ptr<Skybox>     pSkybox_ = nullptr;          // !< スカイボックス

    std::unique_ptr<StageManager> pStageManager_ = nullptr;  // !< ステージ管理クラス
    std::unique_ptr<PauseMenu>    pPauseMenu_ = nullptr;     // !< ポーズメニュー
    std::unique_ptr<InGameUI>     pInGameUI_ = nullptr;      // !< インゲームUI
    bool                          isPaused_ = false;         // !< ポーズ中フラグ
    bool                          isChangingScene_ = false;  // !< シーン遷移中フラグ

    /// 他クラスのインスタンス参照
    PostEffectExecutor* pPostEffectExecutor_ = nullptr;
    DirectX12* pDx12_ = nullptr;
    Input* pInput_ = nullptr;
    SceneManager* pSceneManager_ = nullptr;
    CubemapSystem* pCubemapSystem_ = nullptr;
    InputMapper<InputActionUI>* pInputMapperUI_ = nullptr;
};
