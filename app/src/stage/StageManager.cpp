#include "StageManager.h"
#include <drawable/sprite/SpriteSystem.h>
#include <Core/DirectX12/TextureManager.h>
#include <config/ResourcePath.h>
#include <logic/ConvertNumTex/ConvertNumTex.h>
#include <object/ObjectRule.h>
#include <NiGui.h>
#include <filesystem>
#include <algorithm>
#include <dinput.h>

StageManager::StageManager()
{
    pJSONIO_ = JSONIO::GetInstance();
}

StageManager::~StageManager()
{
}

void StageManager::Initialize()
{
    TextureManager* tm = TextureManager::GetInstance();
    tm->LoadTexture(Path::Image::InGame::kTile);
    tm->LoadTexture(Path::Image::InGame::kTestTile);

    // マップファイル一覧の取得と初期マップの読み込み
    RefreshMapFileList();
    std::string initialMap = "test_map.json";
    if (!mapFileList_.empty())
    {
        bool hasTestMap = false;
        for (const auto& file : mapFileList_)
        {
            if (file == "test_map.json")
            {
                hasTestMap = true;
                break;
            }
        }
        if (!hasTestMap)
        {
            initialMap = mapFileList_[0];
        }
    }
    currentLoadedMapFile_ = initialMap;
    MapLoad(initialMap);

    // スプライトの初期化
    InitializeSprites();

    // ステージ開始時の初期状態を保存
    SaveInitialSnapshot();
}

void StageManager::Update(Input* pInput)
{
    // 地形タイルの更新
    for (auto& tileRow : pSpriteTile_)
    {
        for (auto& tile : tileRow)
        {
            if (tile)
            {
                tile->Update();
            }
        }
    }

    // キーボードによるステージリセット (R) と アンドゥ (Z / U)
    if (pInput && !ImGui::GetIO().WantCaptureKeyboard)
    {
        if (pInput->TriggerKey(DIK_R) || pInput->TriggerKeyC('R') || pInput->TriggerKeyC('r'))
        {
            ResetStage();
        }
        else if (pInput->TriggerKey(DIK_Z) || pInput->TriggerKeyC('Z') || pInput->TriggerKeyC('z') ||
                 pInput->TriggerKey(DIK_U) || pInput->TriggerKeyC('U') || pInput->TriggerKeyC('u'))
        {
            Undo();
        }
    }

    // プレイヤーのキー入力制御 (WASD移動・押し出し・長押し連続移動)
    if (pPlayer_ && pInput)
    {
        GameStepSnapshot snapshotBeforeMove = CaptureSnapshot();
        if (pPlayer_->HandleInput(pInput, mapCollision_, currentMap_, pMapObjects_))
        {
            undoStack_.push_back(snapshotBeforeMove);
            if (undoStack_.size() > 100)
            {
                undoStack_.erase(undoStack_.begin());
            }
        }
    }

    // 全オブジェクトの座標およびアニメーション更新
    for (auto& object : pMapObjects_)
    {
        if (object)
        {
            object->UpdateSpritePosition(tileSize_, mapOffset_);
            object->Update();
        }
    }

    // オブジェクトの移動に合わせて最新の合成マップ（マップ番号）を更新
    UpdateCurrentMap();

    // 電波伝搬・減衰の計算 (初期強度 10 マス)
    isPcConnected_ = signalSystem_.UpdateSignal(mapWidth_, mapHeight_, currentMap_, pMapObjects_, signalStrengthMap_, pcReceivedStrength_);

    // PCオブジェクトへ電波受信状態と強度の通知
    for (auto& object : pMapObjects_)
    {
        if (object && object->GetObjectType() == ObjectType2d::kPC)
        {
            PC* pcObj = static_cast<PC*>(object.get());
            pcObj->UpdateSignal(isPcConnected_, pcReceivedStrength_);
        }
    }

    // 電波表示タイルの更新 (オブジェクトが乗っていない床マスのみ電波を描画)
    for (int y = 0; y < mapHeight_; ++y)
    {
        for (int x = 0; x < mapWidth_; ++x)
        {
            int strength = (y < static_cast<int>(signalStrengthMap_.size()) && x < static_cast<int>(signalStrengthMap_[y].size())) ? signalStrengthMap_[y][x] : 0;
            if (pSignalSpriteTile_[y][x])
            {
                // そのマスにオブジェクト(中継器・ルーター・プレイヤー等)が存在するかチェック
                bool hasObjectOnTile = false;
                for (const auto& obj : pMapObjects_)
                {
                    if (obj && obj->GetPosition() == Vector2Int{ x, y })
                    {
                        hasObjectOnTile = true;
                        break;
                    }
                }

                // 電波が存在し、かつオブジェクトが乗っていない床マスのみ電波を描画！
                bool showSignal = (strength > 0) && !hasObjectOnTile;
                pSignalSpriteTile_[y][x]->SetEnableDraw(showSignal);

                if (showSignal)
                {
                    // 強度 1~10 を 0.0f~1.0f に正規化
                    float norm = static_cast<float>(strength - 1) / 9.0f;
                    if (norm < 0.0f) norm = 0.0f;
                    if (norm > 1.0f) norm = 1.0f;

                    // 10段階で濃い黄金色 (norm=1) ➔ 薄いレモンイエロー (norm=0) へ線形補間
                    float r = 1.0f;
                    float g = 0.96f - 0.16f * norm; // S=10: 0.80, S=1: 0.96
                    float b = 0.45f - 0.40f * norm; // S=10: 0.05, S=1: 0.45
                    float a = 0.16f + 0.72f * norm; // S=10: 0.88, S=1: 0.16

                    pSignalSpriteTile_[y][x]->SetColor({ r, g, b, a });
                    pSignalSpriteTile_[y][x]->SetPosition({ tileSize_ / 2.0f + tileSize_ * x + mapOffset_.x, tileSize_ / 2.0f + tileSize_ * y + mapOffset_.y });
                    pSignalSpriteTile_[y][x]->SetSize({ tileSize_ * 0.70f, tileSize_ * 0.70f });
                    pSignalSpriteTile_[y][x]->Update();
                }
            }
        }
    }

    // エディタの更新
    if (isEditorEnabled_)
    {
        MapEdit();
    }
}

void StageManager::Draw()
{
    // 1. 最背面: 床・壁タイルを描画
    for (auto& tileRow : pSpriteTile_)
    {
        for (auto& tile : tileRow)
        {
            if (tile)
            {
                tile->Draw1F();
            }
        }
    }

    // 2. 中間: 電波の通過レイヤを描画
    for (auto& signalRow : pSignalSpriteTile_)
    {
        for (auto& signalTile : signalRow)
        {
            if (signalTile && signalTile->GetEnableDraw())
            {
                signalTile->Draw1F();
            }
        }
    }

    // 3. 前面: オブジェクトを描画
    for (auto& object : pMapObjects_)
    {
        if (object)
        {
            object->Draw();
        }
    }
}

bool StageManager::IsCleared() const
{
    for (const auto& obj : pMapObjects_)
    {
        if (obj && obj->GetObjectType() == ObjectType2d::kPC)
        {
            const PC* pc = static_cast<const PC*>(obj.get());
            if (pc->IsCleared())
            {
                return true;
            }
        }
    }
    return false;
}

void StageManager::UpdateTileSprite(int x, int y)
{
    int tileType = mapData_[y][x];
    const char* texturePath = ConvertNumTex(tileType);

    pSpriteTile_[y][x]->Initialize(texturePath);
    pSpriteTile_[y][x]->SetName("Tile" + std::to_string(y * mapWidth_ + x));
    pSpriteTile_[y][x]->SetAnchorPoint({ 0.5f, 0.5f });
    pSpriteTile_[y][x]->SetPosition({ tileSize_ / 2.0f + tileSize_ * x + mapOffset_.x, tileSize_ / 2.0f + tileSize_ * y + mapOffset_.y });
    pSpriteTile_[y][x]->SetSize({ tileSize_, tileSize_ });
}

bool StageManager::IsWall(int x, int y) const
{
    if (x < 0 || x >= mapWidth_ || y < 0 || y >= mapHeight_)
    {
        return false;
    }
    int tileType = mapData_[y][x];
    return (tileType >= ObjectRule::kMapWallType && tileType < ObjectRule::kDynamicObjectType);
}

Path::Image::InGame::WallType StageManager::CalculateAutoWallType(int x, int y) const
{
    enum WallDirBit
    {
        kDirLeft   = 1 << 0, // 1
        kDirRight  = 1 << 1, // 2
        kDirTop    = 1 << 2, // 4
        kDirBottom = 1 << 3, // 8
    };

    int mask = 0;
    if (IsWall(x - 1, y)) mask |= kDirLeft;
    if (IsWall(x + 1, y)) mask |= kDirRight;
    if (IsWall(x, y - 1)) mask |= kDirTop;
    if (IsWall(x, y + 1)) mask |= kDirBottom;

    using namespace Path::Image::InGame;
    static const WallType kMaskToWallType[16] = {
        WallType::kT,    // 0: 孤立壁 -> wall_end_T (指定仕様)
        WallType::kL,    // 1: L
        WallType::kR,    // 2: R
        WallType::kRL,   // 3: L + R
        WallType::kT,    // 4: T
        WallType::kLT,   // 5: L + T
        WallType::kRT,   // 6: R + T
        WallType::kLRT,  // 7: L + R + T
        WallType::kB,    // 8: B
        WallType::kLB,   // 9: L + B
        WallType::kRB,   // 10: R + B
        WallType::kLRB,  // 11: L + R + B
        WallType::kTB,   // 12: T + B
        WallType::kLBT,  // 13: L + T + B
        WallType::kRBT,  // 14: R + T + B
        WallType::kAll,  // 15: L + R + T + B
    };

    return kMaskToWallType[mask & 0x0F];
}

void StageManager::UpdateAutoWall(int x, int y)
{
    if (!IsWall(x, y))
    {
        return;
    }

    Path::Image::InGame::WallType wallType = CalculateAutoWallType(x, y);
    int targetTileType = ObjectRule::kMapWallType + static_cast<int>(wallType);
    if (mapData_[y][x] != targetTileType)
    {
        mapData_[y][x] = targetTileType;
        UpdateTileSprite(x, y);
    }
}

void StageManager::UpdateAutoWallWithNeighbors(int x, int y)
{
    if (IsWall(x, y))
    {
        UpdateAutoWall(x, y);
    }

    static const Vector2Int kDirs[] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };
    for (const auto& dir : kDirs)
    {
        int nx = x + dir.x;
        int ny = y + dir.y;
        if (IsWall(nx, ny))
        {
            UpdateAutoWall(nx, ny);
        }
    }
}

void StageManager::UpdateAllAutoWalls()
{
    for (int y = 0; y < mapHeight_; ++y)
    {
        for (int x = 0; x < mapWidth_; ++x)
        {
            if (IsWall(x, y))
            {
                UpdateAutoWall(x, y);
            }
        }
    }
}

void StageManager::InitializeSprites()
{
    TextureManager* tm = TextureManager::GetInstance();
    tm->LoadTexture(Path::Image::InGame::kTestTile);

    pSpriteTile_.resize(mapHeight_);
    pSignalSpriteTile_.resize(mapHeight_);

    for (int y = 0; y < mapHeight_; ++y)
    {
        pSpriteTile_[y].resize(mapWidth_);
        pSignalSpriteTile_[y].resize(mapWidth_);

        for (int x = 0; x < mapWidth_; ++x)
        {
            if (!pSpriteTile_[y][x])
            {
                pSpriteTile_[y][x] = std::make_unique<Sprite>();
                UpdateTileSprite(x, y);

                pSignalSpriteTile_[y][x] = std::make_unique<Sprite>();
                pSignalSpriteTile_[y][x]->Initialize(Path::Image::InGame::kTestTile);
                pSignalSpriteTile_[y][x]->SetAnchorPoint({ 0.5f, 0.5f });
                pSignalSpriteTile_[y][x]->SetColor({ 1.0f, 0.8f, 0.05f, 0.45f }); // 電波用黄色（半透明）
                pSignalSpriteTile_[y][x]->SetPosition({ tileSize_ / 2.0f + tileSize_ * x + mapOffset_.x, tileSize_ / 2.0f + tileSize_ * y + mapOffset_.y });
                pSignalSpriteTile_[y][x]->SetSize({ tileSize_ * 0.70f, tileSize_ * 0.70f });
            }
        }
    }

    for (auto& object : pMapObjects_)
    {
        if (object)
        {
            object->UpdateSpritePosition(tileSize_, mapOffset_);
        }
    }
}

BaseObject2d* StageManager::CreateObject(ObjectType2d type, const Vector2Int& pos, const Vector2Int& dir)
{
    // 指定位置に既存のオブジェクトがあれば削除
    RemoveObjectAt(pos);

    std::unique_ptr<BaseObject2d> newObj = nullptr;

    switch (type)
    {
    case ObjectType2d::kPlayer:
        newObj = std::make_unique<Player>();
        break;
    case ObjectType2d::kRouter:
        newObj = std::make_unique<Router>();
        break;
    case ObjectType2d::kRepeater:
        newObj = std::make_unique<Repeater>();
        break;
    case ObjectType2d::kAlumiWall:
        newObj = std::make_unique<AlumiWall>();
        break;
    case ObjectType2d::kPC:
        newObj = std::make_unique<PC>();
        break;
    default:
        return nullptr;
    }

    if (newObj)
    {
        newObj->SetPosition(pos);
        newObj->SetAngle(dir);
        newObj->Initialize();
        newObj->UpdateSpritePosition(tileSize_, mapOffset_);

        BaseObject2d* ptr = newObj.get();
        if (type == ObjectType2d::kPlayer)
        {
            pPlayer_ = static_cast<Player*>(ptr);
        }

        pMapObjects_.push_back(std::move(newObj));
        UpdateCurrentMap();
        return ptr;
    }

    return nullptr;
}

void StageManager::RemoveObjectAt(const Vector2Int& pos)
{
    for (auto it = pMapObjects_.begin(); it != pMapObjects_.end(); ++it)
    {
        if (*it && (*it)->GetPosition() == pos)
        {
            if ((*it).get() == pPlayer_)
            {
                pPlayer_ = nullptr;
            }
            pMapObjects_.erase(it);
            break;
        }
    }
    UpdateCurrentMap();
}

void StageManager::MapSave(const std::string& path)
{
    nlohmann::json rootJson;
    rootJson["tiles"] = mapData_;

    nlohmann::json objectsJson = nlohmann::json::array();
    for (const auto& obj : pMapObjects_)
    {
        if (obj)
        {
            nlohmann::json objJson;
            objJson["type"] = static_cast<int>(obj->GetObjectType());
            objJson["x"] = obj->GetPosition().x;
            objJson["y"] = obj->GetPosition().y;
            objJson["dirX"] = obj->GetAngle().x;
            objJson["dirY"] = obj->GetAngle().y;
            objectsJson.push_back(objJson);
        }
    }
    rootJson["objects"] = objectsJson;

    pJSONIO_->Save(path, rootJson);
}

void StageManager::InitializeTestObjects()
{
    pMapObjects_.clear();
    pPlayer_ = nullptr;

    // テスト用の初期オブジェクト配置 (ルーター:右向き, 中継器:下向き)
    CreateObject(ObjectType2d::kPlayer, { 2, 2 }, { 0, 1 });
    CreateObject(ObjectType2d::kRouter, { 1, 1 }, { 1, 0 }); // 右向きルーター
    CreateObject(ObjectType2d::kRepeater, { 5, 1 }, { 0, 1 }); // 下向き中継器
    CreateObject(ObjectType2d::kAlumiWall, { 4, 4 }, { 0, 1 });
    CreateObject(ObjectType2d::kPC, { 5, 7 }, { 0, 1 });
}

void StageManager::MapLoad(const std::string& path)
{
    savePath_ = std::string(Path::Resource::kJsonDir) + Path::Json::kMapDir + path.c_str();
    nlohmann::json mapJson = pJSONIO_->Load(savePath_);

    pMapObjects_.clear();
    pPlayer_ = nullptr;

    bool hasObjectsInJson = false;

    if (mapJson.contains("tiles"))
    {
        // 拡張形式 (tiles + objects)
        nlohmann::json tilesJson = mapJson["tiles"];
        mapHeight_ = static_cast<int>(tilesJson.size());
        mapWidth_ = mapHeight_ > 0 ? static_cast<int>(tilesJson[0].size()) : 0;

        mapData_.resize(mapHeight_);
        for (int y = 0; y < mapHeight_; ++y)
        {
            mapData_[y].resize(mapWidth_);
            for (int x = 0; x < mapWidth_; ++x)
            {
                mapData_[y][x] = tilesJson[y][x].get<int>();
            }
        }

        if (mapJson.contains("objects") && mapJson["objects"].is_array() && !mapJson["objects"].empty())
        {
            hasObjectsInJson = true;
            for (const auto& objJson : mapJson["objects"])
            {
                ObjectType2d type = static_cast<ObjectType2d>(objJson["type"].get<int>());
                int x = objJson["x"].get<int>();
                int y = objJson["y"].get<int>();
                Vector2Int dir = { 0, 1 };
                if (objJson.contains("dirX") && objJson.contains("dirY"))
                {
                    dir.x = objJson["dirX"].get<int>();
                    dir.y = objJson["dirY"].get<int>();
                }
                CreateObject(type, { x, y }, dir);
            }
        }
    }
    else
    {
        // 従来形式 (単純な2次元配列)
        mapHeight_ = static_cast<int>(mapJson.size());
        mapWidth_ = mapHeight_ > 0 ? static_cast<int>(mapJson[0].size()) : 0;

        mapData_.resize(mapHeight_);
        for (int y = 0; y < mapHeight_; ++y)
        {
            mapData_[y].resize(mapWidth_);
            for (int x = 0; x < mapWidth_; ++x)
            {
                mapData_[y][x] = mapJson[y][x].get<int>();
            }
        }
    }

    // JSONにオブジェクト情報がなかった場合はテスト用配置を作成して最新形式で保存
    if (!hasObjectsInJson)
    {
        InitializeTestObjects();
        MapSave(savePath_);
    }

    UpdateCurrentMap();
}

void StageManager::UpdateCurrentMap()
{
    currentMap_ = mapData_;
    for (auto&& object : pMapObjects_)
    {
        Vector2Int position = object->GetPosition();
        if (position.y >= 0 && static_cast<size_t>(position.y) < currentMap_.size() &&
            position.x >= 0 && static_cast<size_t>(position.x) < currentMap_[position.y].size())
        {
            currentMap_[position.y][position.x] = object->IsDynamic() ?
                static_cast<int>(object->GetObjectType()) + ObjectRule::kDynamicObjectType :
                static_cast<int>(object->GetObjectType()) + ObjectRule::kStaticObjectType;
        }
    }
}

void StageManager::RefreshMapFileList()
{
    mapFileList_.clear();
    std::string dirPath = std::string(Path::Resource::kJsonDir) + Path::Json::kMapDir;

    std::filesystem::path dir(dirPath);
    if (!std::filesystem::exists(dir))
    {
        std::filesystem::create_directories(dir);
    }

    for (const auto& entry : std::filesystem::directory_iterator(dir))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".json")
        {
            mapFileList_.push_back(entry.path().filename().string());
        }
    }

    std::sort(mapFileList_.begin(), mapFileList_.end());

    selectedMapFileIndex_ = 0;
    for (size_t i = 0; i < mapFileList_.size(); ++i)
    {
        if (mapFileList_[i] == currentLoadedMapFile_)
        {
            selectedMapFileIndex_ = static_cast<int>(i);
            break;
        }
    }
}

void StageManager::CreateNewMap(const std::string& fileName, int width, int height)
{
    std::string actualFileName = fileName;
    if (actualFileName.empty())
    {
        actualFileName = "new_map";
    }
    if (actualFileName.length() < 5 || actualFileName.substr(actualFileName.length() - 5) != ".json")
    {
        actualFileName += ".json";
    }

    mapWidth_ = (std::max)(1, width);
    mapHeight_ = (std::max)(1, height);

    mapData_.assign(mapHeight_, std::vector<int>(mapWidth_, 0));
    signalStrengthMap_.assign(mapHeight_, std::vector<int>(mapWidth_, 0));

    InitializeTestObjects();

    currentLoadedMapFile_ = actualFileName;
    savePath_ = std::string(Path::Resource::kJsonDir) + Path::Json::kMapDir + actualFileName;

    MapSave(savePath_);

    pSpriteTile_.clear();
    pSignalSpriteTile_.clear();
    InitializeSprites();
    UpdateCurrentMap();

    SaveInitialSnapshot();
    RefreshMapFileList();
}

void StageManager::LoadMap(const std::string& fileName)
{
    currentLoadedMapFile_ = fileName;
    MapLoad(fileName);

    pSpriteTile_.clear();
    pSignalSpriteTile_.clear();
    InitializeSprites();

    SaveInitialSnapshot();
    RefreshMapFileList();
}

StageManager::GameStepSnapshot StageManager::CaptureSnapshot() const
{
    GameStepSnapshot snapshot;
    snapshot.objectSnapshots.reserve(pMapObjects_.size());

    for (const auto& obj : pMapObjects_)
    {
        if (obj)
        {
            ObjectSnapshot s;
            s.position = obj->GetPosition();
            s.angle = obj->GetAngle();
            if (obj->GetObjectType() == ObjectType2d::kPC)
            {
                PC* pc = static_cast<PC*>(obj.get());
                s.pcDataProgress = pc->GetDataProgress();
                s.pcIsCleared = pc->IsCleared();
            }
            snapshot.objectSnapshots.push_back(s);
        }
    }
    return snapshot;
}

void StageManager::RestoreSnapshot(const GameStepSnapshot& snapshot)
{
    if (snapshot.objectSnapshots.size() != pMapObjects_.size())
    {
        return;
    }

    for (size_t i = 0; i < pMapObjects_.size(); ++i)
    {
        auto& obj = pMapObjects_[i];
        if (obj)
        {
            const auto& s = snapshot.objectSnapshots[i];
            obj->SetPosition(s.position);
            obj->SetAngle(s.angle);
            obj->UpdateSpritePosition(tileSize_, mapOffset_);

            if (obj->GetObjectType() == ObjectType2d::kPlayer)
            {
                Player* player = static_cast<Player*>(obj.get());
                player->ForceUpdateTexture();
            }
            else if (obj->GetObjectType() == ObjectType2d::kRouter || obj->GetObjectType() == ObjectType2d::kRepeater)
            {
                obj->ApplyRotationToSprite();
            }
            else
            {
                obj->ResetRotation();
            }

            if (obj->GetObjectType() == ObjectType2d::kPC)
            {
                PC* pc = static_cast<PC*>(obj.get());
                pc->SetProgress(s.pcDataProgress, s.pcIsCleared);
            }
        }
    }

    UpdateCurrentMap();
}

void StageManager::SaveInitialSnapshot()
{
    initialSnapshot_ = CaptureSnapshot();
    undoStack_.clear();
}

void StageManager::Undo()
{
    if (pPlayer_)
    {
        pPlayer_->ResetHoldState();
    }

    if (undoStack_.empty())
    {
        return;
    }

    GameStepSnapshot prevSnapshot = undoStack_.back();
    undoStack_.pop_back();

    RestoreSnapshot(prevSnapshot);
}

void StageManager::ResetStage()
{
    if (pPlayer_)
    {
        pPlayer_->ResetHoldState();
    }

    RestoreSnapshot(initialSnapshot_);
    undoStack_.clear();
}

void StageManager::MapEdit()
{
#ifdef _DEBUG
    ImGui::Begin("Map Editor");

    // 電波の接続ステータスを表示
    if (isPcConnected_)
    {
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.3f, 1.0f), "[SIGNAL STATUS]: PC CONNECTED");
    }
    else
    {
        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.3f, 1.0f), "[SIGNAL STATUS]: NO SIGNAL REACHED PC");
    }

    // アンドゥ & ステージリセット
    std::string undoLabel = "Undo (Z) [" + std::to_string(undoStack_.size()) + "]";
    if (ImGui::Button(undoLabel.c_str()))
    {
        Undo();
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset Stage (R)"))
    {
        ResetStage();
    }

    ImGui::Separator();

    bool isMapEditedThisFrame = false;

    // --- 1. 壁・タイルの種類 ---
    static const char* kWallTypeNames[] = {
        "Simple Tile (Floor)",
        "Wall (Auto)",
        "Manual: All", "Manual: LB", "Manual: LBT", "Manual: LRB", "Manual: LRT",
        "Manual: LT", "Manual: RB", "Manual: RBT", "Manual: RL", "Manual: RT",
        "Manual: TB", "Manual: B", "Manual: L", "Manual: R", "Manual: T"
    };

    // モード選択（0: 地形ペイント, 1: オブジェクト配置）
    static int editorMode = 0;
    ImGui::RadioButton("Terrain Mode", &editorMode, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Object Mode", &editorMode, 1);

    ImGui::Separator();

    static int selectedWallIdx = 1; // デフォルトで Wall (Auto) を選択
    static int selectedObjectType = 1; // 1: Player, 2: Router, 3: Repeater, 4: AlumiWall, 5: PC, 0: Remove
    static int selectedDirIdx = 2;     // 0: Up (0,-1), 1: Right (1,0), 2: Down (0,1), 3: Left (-1,0)

    static const Vector2Int kDirVectors[] = { {0, -1}, {1, 0}, {0, 1}, {-1, 0} };
    static const char* kDirNames[] = { "Up (0, -1)", "Right (1, 0)", "Down (0, 1)", "Left (-1, 0)" };

    if (editorMode == 0)
    {
        ImGui::Text("Terrain Palette");
        ImGui::Combo("Wall Type", &selectedWallIdx, kWallTypeNames, IM_ARRAYSIZE(kWallTypeNames));
        if (ImGui::Button("Update All Walls (Auto-Tile)"))
        {
            UpdateAllAutoWalls();
            MapSave(savePath_);
            isMapEditedThisFrame = true;
        }
    }
    else
    {
        ImGui::Text("Object Palette");
        static const char* kObjectNames[] = { "Delete", "Player (Blue)", "Router (Yellow)", "Repeater (Green)", "AlumiWall (Gray)", "PC (Purple)" };
        ImGui::Combo("Object Type", &selectedObjectType, kObjectNames, IM_ARRAYSIZE(kObjectNames));

        if (selectedObjectType == 2 || selectedObjectType == 3) // Router または Repeater の場合、向き設定を表示
        {
            ImGui::Combo("Placement Direction", &selectedDirIdx, kDirNames, IM_ARRAYSIZE(kDirNames));
        }
    }

    ImGui::Separator();
    ImGui::Text("Map Grid (Click to paint/place, Right-Click to Rotate)");

    for (int y = 0; y < mapHeight_; ++y)
    {
        for (int x = 0; x < mapWidth_; ++x)
        {
            ImGui::PushID(y * mapWidth_ + x);

            // マスに存在するオブジェクトがあるか確認
            BaseObject2d* objAtPos = nullptr;
            for (const auto& obj : pMapObjects_)
            {
                if (obj && obj->GetPosition() == Vector2Int{ x, y })
                {
                    objAtPos = obj.get();
                    break;
                }
            }

            // ラベル表示
            std::string label;
            if (objAtPos)
            {
                switch (objAtPos->GetObjectType())
                {
                case ObjectType2d::kPlayer:    label = " P"; break;
                case ObjectType2d::kRouter:    label = " R"; break;
                case ObjectType2d::kRepeater:  label = " M"; break;
                case ObjectType2d::kAlumiWall:label = " W"; break;
                case ObjectType2d::kPC:       label = " C"; break;
                default: label = " O"; break;
                }
            }
            else
            {
                label = (mapData_[y][x] == 0) ? " ." : std::to_string(mapData_[y][x] - ObjectRule::kMapWallType);
            }

            ImGui::Button(label.c_str(), ImVec2(28, 28));

            // 左クリックで配置・操作
            if (ImGui::IsItemHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left))
            {
                if (editorMode == 0) // 地形ペイント
                {
                    if (selectedWallIdx == 0) // 床（消去）
                    {
                        if (mapData_[y][x] != 0)
                        {
                            mapData_[y][x] = 0;
                            UpdateTileSprite(x, y);
                            // 周囲の壁の接続状態が変わるため、隣接壁を再計算
                            UpdateAutoWallWithNeighbors(x, y);
                            isMapEditedThisFrame = true;
                        }
                    }
                    else if (selectedWallIdx == 1) // Auto Wall
                    {
                        if (!IsWall(x, y))
                        {
                            mapData_[y][x] = ObjectRule::kMapWallType;
                            UpdateAutoWallWithNeighbors(x, y);
                            isMapEditedThisFrame = true;
                        }
                        else
                        {
                            // 既に壁の場合でも接続の整合性を更新
                            UpdateAutoWallWithNeighbors(x, y);
                        }
                    }
                    else // 手動指定 (selectedWallIdx >= 2)
                    {
                        int manualWallTypeIdx = selectedWallIdx - 2;
                        int targetTileType = manualWallTypeIdx + ObjectRule::kMapWallType;
                        if (mapData_[y][x] != targetTileType)
                        {
                            mapData_[y][x] = targetTileType;
                            UpdateTileSprite(x, y);
                            isMapEditedThisFrame = true;
                        }
                    }
                }
                else // オブジェクト配置
                {
                    ObjectType2d targetObjType = static_cast<ObjectType2d>(selectedObjectType);
                    if (targetObjType == ObjectType2d::None)
                    {
                        RemoveObjectAt({ x, y });
                    }
                    else
                    {
                        CreateObject(targetObjType, { x, y }, kDirVectors[selectedDirIdx]);
                    }
                    isMapEditedThisFrame = true;
                }
            }
            // 右クリックで既存オブジェクトの向きを時計回りに回転
            else if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
            {
                if (objAtPos)
                {
                    Vector2Int curDir = objAtPos->GetAngle();
                    Vector2Int nextDir = { 0, 1 };
                    if (curDir.x == 0 && curDir.y == -1) nextDir = { 1, 0 };      // 上 -> 右
                    else if (curDir.x == 1 && curDir.y == 0) nextDir = { 0, 1 };  // 右 -> 下
                    else if (curDir.x == 0 && curDir.y == 1) nextDir = { -1, 0 }; // 下 -> 左
                    else if (curDir.x == -1 && curDir.y == 0) nextDir = { 0, -1 };// 左 -> 上

                    objAtPos->SetAngle(nextDir);
                    isMapEditedThisFrame = true;
                }
            }

            ImGui::PopID();

            if (x < mapWidth_ - 1)
            {
                ImGui::SameLine();
            }
        }
    }

    if ((ImGui::IsMouseReleased(ImGuiMouseButton_Left) || ImGui::IsMouseReleased(ImGuiMouseButton_Right)) && isMapEditedThisFrame)
    {
        MapSave(savePath_);
        SaveInitialSnapshot();
    }

    ImGui::Separator();
    ImGui::Text("Map File Management");

    // 1. 現在のマップ情報 & 上書き保存
    ImGui::Text("Current Map: %s", currentLoadedMapFile_.c_str());
    ImGui::SameLine();
    if (ImGui::Button("Save Current Map"))
    {
        MapSave(savePath_);
    }

    // 2. 既存マップの選択読み込み
    if (!mapFileList_.empty())
    {
        std::vector<const char*> fileCStrs;
        fileCStrs.reserve(mapFileList_.size());
        for (const auto& file : mapFileList_)
        {
            fileCStrs.push_back(file.c_str());
        }

        if (selectedMapFileIndex_ < 0 || selectedMapFileIndex_ >= static_cast<int>(fileCStrs.size()))
        {
            selectedMapFileIndex_ = 0;
        }

        ImGui::SetNextItemWidth(180);
        ImGui::Combo("Select Map", &selectedMapFileIndex_, fileCStrs.data(), static_cast<int>(fileCStrs.size()));
        ImGui::SameLine();
        if (ImGui::Button("Load Map"))
        {
            LoadMap(mapFileList_[selectedMapFileIndex_]);
            isMapEditedThisFrame = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Refresh List"))
        {
            RefreshMapFileList();
        }
    }
    else
    {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.2f, 1.0f), "No map files found.");
        ImGui::SameLine();
        if (ImGui::Button("Refresh List"))
        {
            RefreshMapFileList();
        }
    }

    // 3. ファイル名を指定して新規作成
    ImGui::SetNextItemWidth(150);
    ImGui::InputText("New File Name", newMapNameInput_, sizeof(newMapNameInput_));
    ImGui::SameLine();
    ImGui::SetNextItemWidth(50);
    ImGui::InputInt("W##NewW", &newMapWidth_);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(50);
    ImGui::InputInt("H##NewH", &newMapHeight_);
    ImGui::SameLine();
    if (ImGui::Button("Create New Map"))
    {
        CreateNewMap(newMapNameInput_, newMapWidth_, newMapHeight_);
        isMapEditedThisFrame = true;
    }

    ImGui::Separator();
    ImGui::Text("Map Display & Resize");
    ImGui::PushItemWidth(100);

    ImGui::DragFloat2("Map Offset", &mapOffset_.x, 1.0f, -1000.0f, 1000.0f);
    ImGui::SameLine();
    ImGui::DragFloat("Tile Size", &tileSize_, 1.0f, 1.0f, 256.0f);
    if (ImGui::DragInt("Map Width", &mapWidth_, 1.0f, 1, 256))
    {
        for (int y = 0; y < mapHeight_; ++y)
        {
            mapData_[y].resize(mapWidth_, 0);
            signalStrengthMap_[y].resize(mapWidth_, 0);
        }
        UpdateCurrentMap();
        InitializeSprites();
    }
    ImGui::SameLine();
    if (ImGui::DragInt("Map Height", &mapHeight_, 1.0f, 1, 256))
    {
        mapData_.resize(mapHeight_, std::vector<int>(mapWidth_, 0));
        signalStrengthMap_.resize(mapHeight_, std::vector<int>(mapWidth_, 0));
        UpdateCurrentMap();
        InitializeSprites();
    }
    if (ImGui::Button("refresh"))
    {
        for (int y = 0; y < mapHeight_; ++y)
        {
            for (int x = 0; x < mapWidth_; ++x)
            {
                UpdateTileSprite(x, y);
            }
        }
        for (auto& object : pMapObjects_)
        {
            if (object)
            {
                object->UpdateSpritePosition(tileSize_, mapOffset_);
            }
        }
    }
    ImGui::PopItemWidth();

    ImGui::End();
#endif // _DEBUG
}
