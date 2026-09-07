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
    /// ImGuiエディタの有効/無効
    /// </summary>
    void SetEditorEnabled(bool enabled) { isEditorEnabled_ = enabled; }
    bool IsEditorEnabled() const { return isEditorEnabled_; }

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
    BaseObject2d* CreateObject(ObjectType2d type, const Vector2Int& pos, const Vector2Int& dir = { 0, 1 });
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

    MapCollision mapCollision_; // !< 衝突・押し出し判定
    SignalSystem signalSystem_; // !< 電波伝搬システム

    JSONIO* pJSONIO_ = nullptr; // !< JSONIO

    bool isEditorEnabled_ = true; // !< エディタ表示フラグ
};
