#pragma once

#include <memory>
#include <array>
#include <drawable/sprite/Sprite.h>
#include <Features/Input/Input.h>
#include <Features/Input/InputMapper.hpp>
#include <logic/input/InputAction.h>
#include <Vector2.h>
#include <Vector4.h>
#include <Features/Audio/Audio.h>

/// <summary>
/// タイトルメニューのアクション
/// </summary>
enum class TitleMenuAction
{
    None,
    StartGame, // ゲーム開始 (ステージセレクトへ遷移)
    EndGame    // ゲーム終了 (アプリケーション終了)
};

/// <summary>
/// タイトル画面用メニューUIクラス
/// </summary>
class TitleMenu
{
public:
    enum MenuItem
    {
        kStartGame = 0, // ゲームスタート
        kEndGame = 1,   // ゲーム終了
        kItemCount = 2
    };

    TitleMenu();
    ~TitleMenu();

    /// <summary>
    /// 初期化 (スプライトの生成と初期レイアウト設定)
    /// </summary>
    void Initialize();

    /// <summary>
    /// 更新 (入力、カーソル移動、アニメーション)
    /// </summary>
    /// <param name="pInput">Inputインスタンス</param>
    /// <param name="pInputMapper">UI入力マッパー (任意)</param>
    void Update(Input* pInput, InputMapper<InputActionUI>* pInputMapper = nullptr);

    /// <summary>
    /// 描画
    /// </summary>
    void Draw();

    /// <summary>
    /// 決定されたアクションを取得して内部状態をリセットする
    /// </summary>
    TitleMenuAction ConsumeAction();

    /// <summary>
    /// 現在選択中の項目インデックスを取得
    /// </summary>
    int GetSelectedIndex() const { return selectedIndex_; }

private:
    void UpdateLayout();
    void TriggerActionByIndex(int index);

private:
    int selectedIndex_ = kStartGame;
    TitleMenuAction currentAction_ = TitleMenuAction::None;

    // スプライト群
    std::array<std::unique_ptr<Sprite>, kItemCount> pButtonBgSprites_{};   // !< ボタン背景プレート (Simple.png)
    std::array<std::unique_ptr<Sprite>, kItemCount> pButtonTextSprites_{}; // !< ボタン画像 (goGame_text.png, exitGame_text.png)
    std::unique_ptr<Sprite> pCursorBorderSprite_ = nullptr;                // !< 選択中ボタンの外枠ハイライト
    std::unique_ptr<Sprite> pIndicatorLeft_ = nullptr;                     // !< 選択中の左側アクセントバー
    std::unique_ptr<Sprite> pIndicatorRight_ = nullptr;                    // !< 選択中の右側アクセントバー

    Audio* pChoiceAudio_ = nullptr;   // !< カーソル移動SE (kChoiceSE)
    Audio* pDecisionAudio_ = nullptr; // !< 決定SE (kDecisionSE)

    float animTimer_ = 0.0f; // !< 選択中ボタンのパルス・アニメーション用タイマー
};