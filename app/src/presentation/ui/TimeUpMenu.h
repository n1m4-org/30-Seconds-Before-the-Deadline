#pragma once

#include <memory>
#include <array>
#include <drawable/sprite/Sprite.h>
#include <Features/Input/Input.h>
#include <Vector2.h>
#include <Vector4.h>
#include <string>
#include <Features/Audio/Audio.h>

/// <summary>
/// タイムアップ (時間切れ) メニューで実行されたアクション
/// </summary>
enum class TimeUpMenuAction
{
    None,        // アクションなし
    Retry,       // リトライ (現在のステージを再開)
    StageSelect, // セレクトへ (ステージセレクトへ)
    Title        // タイトル画面へ
};

/// <summary>
/// タイムアップ (時間切れ) メニューUIクラス
/// </summary>
class TimeUpMenu
{
public:
    enum MenuItem
    {
        kRetry = 0,       // 1. リトライ
        kStageSelect = 1, // 2. セレクトへ
        kTitle = 2,       // 3. タイトルへ
        kItemCount = 3
    };

    TimeUpMenu();
    ~TimeUpMenu();

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
    /// タイムアップメニューを開く
    /// </summary>
    void Open();

    /// <summary>
    /// タイムアップメニューを閉じる
    /// </summary>
    void Close();

    /// <summary>
    /// 開いているかどうか
    /// </summary>
    bool IsOpen() const { return isOpen_; }

    /// <summary>
    /// ステージタイトルを設定
    /// </summary>
    void SetStageTitle(const std::string& title) { stageTitle_ = title; }

    /// <summary>
    /// 決定されたアクションを取得し、内部状態をリセットする
    /// </summary>
    TimeUpMenuAction ConsumeAction();

private:
    void UpdateLayout();
    void DrawOverlayUI();
    void TriggerActionByIndex(int index);

private:
    bool isOpen_ = false;
    std::string stageTitle_ = "";
    int selectedIndex_ = 0;
    TimeUpMenuAction currentAction_ = TimeUpMenuAction::None;

    std::unique_ptr<Sprite> pOverlaySprite_ = nullptr; // !< 画面全体の半透明暗幕
    std::unique_ptr<Sprite> pPanelSprite_ = nullptr;   // !< メニュー中央のパネル背景
    std::array<std::unique_ptr<Sprite>, kItemCount> pButtonSprites_{}; // !< 各ボタン
    std::array<std::unique_ptr<Sprite>, kItemCount> pTextSprites_{}; // !< 各ボタン
    std::unique_ptr<Sprite> pCursorSprite_ = nullptr;  // !< 選択中ボタンのカーソルバー
    std::unique_ptr<Sprite> pTimeUpSprite_ = nullptr;   // !< クリアテキスト

    Audio* pChoiceAudio_ = nullptr;   // !< カーソル移動SE (kChoiceSE)
    Audio* pDecisionAudio_ = nullptr; // !< 決定SE (kDecisionSE)

    float animTimer_ = 0.0f; // !< 選択中ボタンのパルス・アニメーションタイマー
};
