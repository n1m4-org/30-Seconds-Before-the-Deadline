#pragma once

#include <memory>
#include <array>
#include <drawable/sprite/Sprite.h>
#include <Features/Input/Input.h>
#include <Vector2.h>
#include <Vector4.h>
#include <Features/Audio/Audio.h>

/// <summary>
/// ポーズメニューで実行されたアクション
/// </summary>
enum class PauseMenuAction
{
    None,        // アクションなし
    Resume,      // ゲームへ戻る (ポーズ解除)
    StageSelect, // ステージセレクトへ (仮配置)
    Title        // タイトル画面へ
};

/// <summary>
/// ポーズメニューUIクラス (Simple.png による代用仮配置)
/// </summary>
class PauseMenu
{
public:
    enum MenuItem
    {
        kResume = 0,     // 1. ゲームへ戻る (最上部・最頻用)
        kStageSelect = 1,// 2. ステージセレクトへ (真ん中・仮配置)
        kTitle = 2,      // 3. タイトルへ (最下部・誤操作防止)
        kItemCount = 3
    };

    PauseMenu();
    ~PauseMenu();

    /// <summary>
    /// 初期化 (スプライトの生成と初期レイアウト設定)
    /// </summary>
    void Initialize();

    /// <summary>
    /// 更新 (カーソル移動、決定判定、アニメーション)
    /// </summary>
    /// <param name="pInput">入力インスタンス</param>
    void Update(Input* pInput);

    /// <summary>
    /// 描画 (オーバーレイ、パネル、ボタン、カーソル、UIテキスト)
    /// </summary>
    void Draw();

    /// <summary>
    /// ポーズメニューを開く
    /// </summary>
    void Open();

    /// <summary>
    /// ポーズメニューを閉じる
    /// </summary>
    void Close();

    /// <summary>
    /// 開いているかどうか
    /// </summary>
    bool IsOpen() const { return isOpen_; }

    /// <summary>
    /// 決定されたアクションを取得し、内部状態をリセットする
    /// </summary>
    PauseMenuAction ConsumeAction();

private:
    void UpdateLayout();
    void DrawOverlayUI();
    void TriggerActionByIndex(int index);

private:
    bool isOpen_ = false;
    int selectedIndex_ = 0;
    PauseMenuAction currentAction_ = PauseMenuAction::None;

    std::unique_ptr<Sprite> pOverlaySprite_ = nullptr; // !< 画面全体の半透明暗幕
    std::unique_ptr<Sprite> pPanelSprite_ = nullptr;   // !< メニュー中央のパネル背景
    std::array<std::unique_ptr<Sprite>, kItemCount> pButtonSprites_{}; // !< 各ボタン
    std::array<std::unique_ptr<Sprite>, kItemCount> pTextSprites_{}; // !< 各ボタン
    std::unique_ptr<Sprite> pCursorSprite_ = nullptr;  // !< 選択中ボタンのカーソルバー

    Audio* pChoiceAudio_ = nullptr;   // !< カーソル移動SE (kChoiceSE)
    Audio* pDecisionAudio_ = nullptr; // !< 決定SE (kDecisionSE)

    float animTimer_ = 0.0f; // !< 選択中ボタンのパルス・アニメーションタイマー
};
