#pragma once

#include <memory>
#include <array>
#include <drawable/sprite/Sprite.h>
#include <Features/Input/Input.h>
#include <Vector2.h>
#include <Vector4.h>

/// <summary>
/// リザルトメニューで実行されたアクション
/// </summary>
enum class ResultMenuAction
{
    None,        // アクションなし
    NextStage,   // 次のステージへ
    StageSelect, // ステージセレクトへ
    Title        // タイトル画面へ
};

/// <summary>
/// リザルトメニューUIクラス (PauseMenuを参考に作成)
/// </summary>
class ResultMenu
{
public:
    enum MenuItem
    {
        kNextStage = 0,   // 1. 次のステージへ
        kStageSelect = 1, // 2. ステージセレクトへ
        kTitle = 2,       // 3. タイトルへ
        kItemCount = 3
    };

    ResultMenu();
    ~ResultMenu();

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
    /// リザルトメニューを開く
    /// </summary>
    void Open();

    /// <summary>
    /// リザルトメニューを閉じる
    /// </summary>
    void Close();

    /// <summary>
    /// 開いているかどうか
    /// </summary>
    bool IsOpen() const { return isOpen_; }

    /// <summary>
    /// 決定されたアクションを取得し、内部状態をリセットする
    /// </summary>
    ResultMenuAction ConsumeAction();

private:
    void UpdateLayout();
    void DrawOverlayUI();
    void TriggerActionByIndex(int index);

private:
    bool isOpen_ = false;
    int selectedIndex_ = 0;
    ResultMenuAction currentAction_ = ResultMenuAction::None;

    std::unique_ptr<Sprite> pOverlaySprite_ = nullptr; // !< 画面全体の半透明暗幕
    std::unique_ptr<Sprite> pPanelSprite_ = nullptr;   // !< メニュー中央のパネル背景
    std::array<std::unique_ptr<Sprite>, kItemCount> pButtonSprites_{}; // !< 各ボタン
    std::unique_ptr<Sprite> pCursorSprite_ = nullptr;  // !< 選択中ボタンのカーソルバー
	std::unique_ptr<Sprite> pClearSprite_ = nullptr;   // !< クリアテキスト

    float animTimer_ = 0.0f; // !< 選択中ボタンのパルス・アニメーションタイマー
};
