#pragma once

#include <scene/SceneBase.h>
#include <Core/DirectX12/DirectX12.h>
#include <Features/Input/Input.h>
#include <Features/SceneManager/SceneManager.h>
#include <Features/Cubemap/CubemapSystem.h>
#include <Features/Layer/Canvas.h>
#include <drawable/sprite/Sprite.h>
#include <Features/GameEye/GameEye2d.h>
#include <Features/Audio/Audio.h>
#include <memory>
#include <vector>
#include <string>

/// <summary>
/// ステージセレクトシーン
/// maps/ フォルダ内の stageW_S.json を動的スキャンして一覧化し、
/// simple.png のカードを並べて選択中のステージを鮮やかにハイライトする
/// </summary>
class SelectScene : public SceneBase
{
public:
    SelectScene(ISceneArgs* pArgs);
    ~SelectScene() override;

    void Initialize() override;
    void Finalize() override;
    void Update() override;
    void Draw() override;

private:
    struct StageCard
    {
        int world = 1;
        int stage = 1;
        std::string fileName;
        std::string displayName;
        Vector2 basePos = {};

        // カードを構成する個別スプライト
        std::unique_ptr<Sprite> pGlow = nullptr;       // 選択時発光外枠 (simple.png)
        std::unique_ptr<Sprite> pBorder = nullptr;     // 外枠 (simple.png)
        std::unique_ptr<Sprite> pBg = nullptr;         // 背景 (simple.png)
        std::unique_ptr<Sprite> pWorldDigit = nullptr; // ワールド数字 (number_x96y128.png)
        std::unique_ptr<Sprite> pHyphen = nullptr;     // ハイフン横棒 (simple.png)
        std::unique_ptr<Sprite> pStageDigit = nullptr; // ステージ数字 (number_x96y128.png)
        std::unique_ptr<Sprite> pPin = nullptr;        // 選択インジケーター (simple.png)
    };

    struct WorldHeader
    {
        int world = 1;
        Vector2 basePos = {};
        std::unique_ptr<Sprite> pBg = nullptr;
        std::unique_ptr<Sprite> pDigit = nullptr;
    };

    void InitializeGameEye();
    void InitializeCanvas();
    void ScanMapFiles();
    void BuildStageCards();
    void UpdateInput();
    void UpdateCardVisuals();

private:
    DirectX12*                  pDx12_          = nullptr;
    Input*                      pInput_         = nullptr;
    SceneManager*               pSceneManager_  = nullptr;
    CubemapSystem*              pCubemapSystem_ = nullptr;

    std::unique_ptr<GameEye2d>  pGameEye_       = nullptr;
    std::unique_ptr<Canvas>     pCanvasUI_      = nullptr;

    // 全体背景スプライト (simple.png)
    std::unique_ptr<Sprite>     pSpriteScreenBg_ = nullptr;

    std::vector<StageCard>      stageCards_;
    std::vector<WorldHeader>    worldHeaders_;
    std::vector<int>            worldList_;

    int                         selectedIndex_  = 0;
    float                       animTimer_      = 0.0f;
    float                       currentScrollY_ = 0.0f;
    float                       targetScrollY_  = 0.0f;
    bool                        isChangingScene_= false;
    Audio* pBgmAudio_ = nullptr; // !< BGMオーディオ
    Audio* pChoiceAudio_ = nullptr; // !< カーソル移動SE (kChoiceSE)
    Audio* pDecisionAudio_ = nullptr; // !< 決定SE (kDecisionSE)
};
