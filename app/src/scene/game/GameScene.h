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
#include <Math/ViewportUnits.hpp>
#include <drawable/sprite/Sprite.h>
#include <logic/input/InputAction.h>
#include <Utility/JSONIO/JSONIO.h>
#include <object/baseObject2d/BaseObject2d.h>
#include <object/player/Player.h>
#include <object/router/Router.h>
#include <object/repeater/Repeater.h>
#include <object/alumiWall/AlumiWall.h>
#include <object/pc/PC.h>
#include <logic/mapCollision/MapCollision.h>
#include <logic/signal/SignalSystem.h>
#include <config/ResourcePath.h>

/// <summary>
/// タイトルシーン
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
    void InitializeSprites();
    void InitializeSkybox();

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
    void LoadMap(const std::string& fileName);

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
    void Undo();
    void ResetStage();
    void SaveInitialSnapshot();


	// 指定タイプのオブジェクトを作成し配置するヘルパー関数
	BaseObject2d* CreateObject(ObjectType2d type, const Vector2Int& pos, const Vector2Int& dir = { 0, 1 });
	// 指定座標にあるオブジェクトを削除する
	void RemoveObjectAt(const Vector2Int& pos);

    std::unique_ptr<Canvas>             pCanvasBack_ = nullptr;      // !< キャンバス
    std::unique_ptr<Canvas>             pCanvasSprite_ = nullptr;      // !< キャンバス
    std::unique_ptr<GameEye>            gameEye_ = {};           // !< ゲームアイ
    std::unique_ptr<Skybox>             pSkybox_ = nullptr;      // !< スカイボックス
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


    /// 他クラスのインスタンス
    PostEffectExecutor* pPostEffectExecutor_ = nullptr;      // !< ポストエフェクト実行クラス
    DirectX12* pDx12_ = nullptr;      // !< DirectX12
    Input* pInput_ = nullptr;      // !< 入力
    SceneManager* pSceneManager_ = nullptr;      // !< シーン遷移
    CubemapSystem* pCubemapSystem_ = nullptr;      // !< キューブマップシステム
    InputMapper<InputActionUI>* pInputMapperUI_ = nullptr;      // !< 入力マッパー
	JSONIO* pJSONIO_ = nullptr;      // !< JSONIO
};
