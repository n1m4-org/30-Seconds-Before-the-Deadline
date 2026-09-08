#pragma once

#include <vector>
#include <memory>
#include <string>
#include <drawable/sprite/Sprite.h>
#include <Features/Input/Input.h>
#include <object/baseObject2d/BaseObject2d.h>
#include <object/player/Player.h>
#include <object/router/Router.h>
#include <object/repeater/Repeater.h>
#include <object/alumiWall/AlumiWall.h>
#include <object/pc/PC.h>
#include <object/rotatingFloor/RotatingFloor.h>
#include <logic/mapCollision/MapCollision.h>
#include <logic/signal/SignalSystem.h>
#include <config/ResourcePath.h>
#include <Utility/JSONIO/JSONIO.h>

/// <summary>
/// ステージ管理クラス
/// マップデータ、配置オブジェクト、電波伝搬、アンドゥ・リセット、マップエディタ等を統括する
/// </summary>
class StageManager
{
public:
    StageManager();
    ~StageManager();

    /// <summary>
    /// 初期化 (初期マップ読み込み、スプライト生成、初期スナップショット保存)
    /// </summary>
    void Initialize();

    /// <summary>
    /// ステージセレクト等から指定された読み込み対象ステージファイル名
    /// </summary>
    static void SetSelectedStageFileName(const std::string& fileName) { sSelectedStageFileName_ = fileName; }
    static const std::string& GetSelectedStageFileName() { return sSelectedStageFileName_; }

    /// <summary>
    /// 直前にプレイしていた（現在滞在中の）ステージファイル名
    /// </summary>
    static void SetLastPlayedStageFileName(const std::string& fileName) { sLastPlayedStageFileName_ = fileName; }
    static const std::string& GetLastPlayedStageFileName() { return sLastPlayedStageFileName_; }

    /// <summary>
    /// 更新 (入力、オブジェクト挙動、電波計算、Undo/Reset、エディタ)
    /// </summary>
    /// <param name="pInput">入力インスタンス</param>
    void Update(Input* pInput);

    /// <summary>
    /// 描画 (床・壁タイル、電波、配置オブジェクト)
    /// </summary>
    void Draw();

    /// <summary>
    /// 指定ファイルからマップを読み込む
    /// </summary>
    void LoadMap(const std::string& fileName);

    /// <summary>
    /// ステージの初期状態へリセット (Rキー)
    /// </summary>
    void ResetStage();

    /// <summary>
    /// 1手アンドゥ (Z / U キー)
    /// </summary>
    void Undo();

    /// <summary>
    /// PCが電波を受信して何%蓄積したか
    /// </summary>
	float GetPcDataProgress() const;

    /// <summary>
    /// ステージクリア状態 (PCが電波を受信して100%蓄積完了したか)
    /// </summary>
    bool IsCleared() const;

    /// <summary>
    /// PCに電波が届いているか
    /// </summary>
    bool IsPcConnected() const { return isPcConnected_; }

    /// <summary>
    /// PCが受信している電波強度
    /// </summary>
    int GetPcReceivedStrength() const { return pcReceivedStrength_; }

    /// <summary>
    /// ステージ制限時間の基準値 (30秒固定)
    /// </summary>
    static constexpr float kDefaultTimeLimit = 30.0f;

    /// <summary>
    /// 残り制限時間を取得 (秒)
    /// </summary>
    float GetRemainingTime() const { return remainingTime_; }

    /// <summary>
    /// 残り制限時間を設定 (秒)
    /// </summary>
    void SetRemainingTime(float time) { remainingTime_ = time; }

    /// <summary>
    /// 制限時間がタイムアップしたかどうか (Playモード時のみ判定)
    /// </summary>
    bool IsTimeUp() const { return (editorState_ == EditorState::Play) && (remainingTime_ <= 0.0f); }

    /// <summary>
    /// 制限時間タイマーの有効/無効
    /// </summary>
    void SetTimeLimitEnabled(bool enabled) { isTimeLimitEnabled_ = enabled; }
    bool IsTimeLimitEnabled() const { return isTimeLimitEnabled_; }

    /// <summary>
    /// マップの描画オフセット設定・取得
    /// </summary>
    void SetMapOffset(const Vector2& offset) { mapOffset_ = offset; }
    const Vector2& GetMapOffset() const { return mapOffset_; }

    /// <summary>
    /// タイルサイズ設定・取得
    /// </summary>
    void SetTileSize(float size) { tileSize_ = size; }
    float GetTileSize() const { return tileSize_; }

    /// <summary>
    /// エディタ状態 (Play: テストプレイ / Edit: マップ編集)
    /// </summary>
    enum class EditorState
    {
        Play, // プレイモード (ゲームプレイ実行・操作・タイマー動作)
        Edit  // エディットモード (マップ編集・操作およびタイマー停止)
    };

    /// <summary>
    /// エディタ状態の設定・取得
    /// </summary>
    void SetEditorState(EditorState state);
    EditorState GetEditorState() const { return editorState_; }
    bool IsPlayMode() const { return editorState_ == EditorState::Play; }
    bool IsEditMode() const { return editorState_ == EditorState::Edit; }
    void ToggleEditorState();

    /// <summary>
    /// ImGuiエディタの有効/無効
    /// </summary>
    void SetEditorEnabled(bool enabled) { isEditorEnabled_ = enabled; }
    bool IsEditorEnabled() const { return isEditorEnabled_; }

    /// <summary>
    /// 現在読み込み中のマップファイル名を取得
    /// </summary>
    const std::string& GetCurrentLoadedMapFile() const { return currentLoadedMapFile_; }

    /// <summary>
    /// ステージファイル名 (stageW_S.json) をパースしてワールド番号・ステージ番号を取得
    /// </summary>
    bool ParseStageFileName(const std::string& fileName, int& outWorld, int& outStage) const;

    /// <summary>
    /// 次のステージファイル名を取得 (存在しない場合は空文字)
    /// </summary>
    std::string GetNextStageFileName() const;

    /// <summary>
    /// 次のステージが存在するかどうか
    /// </summary>
    bool HasNextStage() const;

    /// <summary>
    /// 次のステージへ進む
    /// </summary>
    /// <returns>次のステージが存在して読み込みに成功したか</returns>
    bool LoadNextStage();

    /// <summary>
    /// 現在のステージの表示名 (例: "STAGE 1-1") を取得
    /// </summary>
    std::string GetCurrentStageDisplayName() const;

    /// <summary>
    /// マップサイズ (幅・高さ) に合わせて画面中央 (900x900領域) に綺麗に収まるよう
    /// タイルサイズと描画オフセットを自動計算・更新する
    /// </summary>
    void UpdateLayoutForMapSize();

private:
    void InitializeSprites();
    void MapEdit();
    void UpdateTileSprite(int x, int y);
    void MapLoad(const std::string& path);
    void MapSave(const std::string& path);
    void UpdateCurrentMap();
    void InitializeTestObjects();

    // 壁のオートタイリング関連ヘルパー関数
    bool IsWall(int x, int y) const;
    Path::Image::InGame::WallType CalculateAutoWallType(int x, int y) const;
    void UpdateAutoWall(int x, int y);
    void UpdateAutoWallWithNeighbors(int x, int y);
    void UpdateAllAutoWalls();

    // マップファイル管理ヘルパー関数
    void RefreshMapFileList();
    void CreateNewMap(const std::string& fileName, int width, int height);

    // ステージリセット & アンドゥ関連
    struct ObjectSnapshot
    {
        Vector2Int position;
        Vector2Int angle;
        RotatingFloorType rotatingFloorType = RotatingFloorType::kRight;
        bool isActive = true;
        float pcDataProgress = 0.0f;
        bool pcIsCleared = false;
    };

    struct GameStepSnapshot
    {
        std::vector<ObjectSnapshot> objectSnapshots;
    };

    GameStepSnapshot CaptureSnapshot() const;
    void RestoreSnapshot(const GameStepSnapshot& snapshot);
    void SaveInitialSnapshot();

    // 指定タイプのオブジェクトを作成し配置するヘルパー関数
    BaseObject2d* CreateObject(ObjectType2d type, const Vector2Int& pos, const Vector2Int& dir = { 0, 1 }, RotatingFloorType floorType = RotatingFloorType::kRight);
    // 指定座標にあるオブジェクトを削除する
    void RemoveObjectAt(const Vector2Int& pos);

private:
    std::vector<std::vector<std::unique_ptr<Sprite>>> pSpriteTile_;         // !< タイル
    std::vector<std::vector<std::unique_ptr<Sprite>>> pSignalSpriteTile_;   // !< 電波可視化用スプライト

    std::vector<std::unique_ptr<BaseObject2d>> pMapObjects_;  // !< マップオブジェクト
    Player* pPlayer_ = nullptr; // プレイヤーの参照キャッシュ

    std::vector<std::vector<int>> mapData_;  // !< 地形マップデータ
    std::vector<std::vector<int>> currentMap_;  // !< 現在のマップ（合成）
    std::vector<std::vector<int>> signalStrengthMap_; // !< 各マスの電波強度 (0~10)
    int pcReceivedStrength_ = 0;               // !< PCが受信した電波の強度
    bool isPcConnected_ = false;               // !< PCに電波が届いているか
    std::string savePath_ = "";                   // !< マップデータ保存先

    // マップファイル管理用変数
    std::vector<std::string> mapFileList_;        // !< maps/ 以下のマップファイル名一覧
    std::string currentLoadedMapFile_ = "test_map.json"; // !< 現在読み込み中のマップファイル名
    int selectedMapFileIndex_ = 0;               // !< UIで選択中のファイルインデックス
    char newMapNameInput_[128] = "new_map";       // !< 新規作成マップ名入力バッファ
    int newMapWidth_ = 9;                         // !< 新規作成マップの幅
    int newMapHeight_ = 9;                        // !< 新規作成マップの高さ

    // ステージリセット & アンドゥ用変数
    std::vector<GameStepSnapshot> undoStack_;   // !< アンドゥ用履歴スタック
    GameStepSnapshot initialSnapshot_;          // !< ステージ開始時の初期状態

    Vector2 mapOffset_ = { 350.0f, 0.0f };   // !< マップのオフセット
    float tileSize_ = 100.0f;   // !< タイルのサイズ
    int mapWidth_ = 9;      // !< マップの幅
    int mapHeight_ = 9;      // !< マップの高さ

    // 制限時間 (30秒固定)
    float remainingTime_ = kDefaultTimeLimit; // !< 残り制限時間 (秒)
    bool isTimeLimitEnabled_ = true;          // !< 制限時間タイマー有効フラグ

    MapCollision mapCollision_; // !< 衝突・押し出し判定
    SignalSystem signalSystem_; // !< 電波伝搬システム

    JSONIO* pJSONIO_ = nullptr; // !< JSONIO

    bool isEditorEnabled_ = true; // !< エディタ表示フラグ
    EditorState editorState_ = EditorState::Play; // !< エディタ状態 (Play / Edit)

    inline static std::string sSelectedStageFileName_ = "";
    inline static std::string sLastPlayedStageFileName_ = "stage1_1.json";
};
