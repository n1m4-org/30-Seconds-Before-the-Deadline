#pragma once
#include <string_view>

namespace Path::Resource
{
    inline constexpr const char* kJsonDir = "resources/json/";
    inline constexpr const char* kTextureDir = "resources/textures/";
    inline constexpr const char* kModelDir = "resources/model/";
    inline constexpr const char* kAudioDir = "resources/sounds/";
    inline constexpr const char* kParticleEmitterDir = "resources/json/particles/";
}

/// <summary>
/// 画像リソースのパスを定義する名前空間
/// </summary>
namespace Path::Image
{
    /// [ 共通 ]
    inline constexpr const char* kWhite = "white1x1.png";
    inline constexpr const char* kNumbers[] =
    {
        "common/font/number_0.dds",
        "common/font/number_1.dds",
        "common/font/number_2.dds",
        "common/font/number_3.dds",
        "common/font/number_4.dds",
        "common/font/number_5.dds",
        "common/font/number_6.dds",
        "common/font/number_7.dds",
        "common/font/number_8.dds",
        "common/font/number_9.dds",
    };
    inline constexpr const char* kCountDownEndString = "common/font/start.dds";
    inline constexpr const char* kSpaceText = "common/font/space.dds";
    inline constexpr const char* kClearText = "common/font/clear.dds";
    inline constexpr const char* kFinishText = "common/font/finish.png";

    namespace Title
    {
        inline constexpr const char* kMenuItemStart = "title/finish.png";
        inline constexpr const char* kMenuItemExit = "title/finish.png";
    }

    inline constexpr const char* kTitleSkybox = "title/skybox.v2.dds";

    /// [ クリア ]
    namespace Clear
    {
        inline constexpr const char* kScoreResult[] =
        {
            "clear/score_s.png",
            "clear/score_a.png",
            "clear/score_b.png",
            "clear/score_c.png",
        };

    }

    enum class PlayerTextureNames
    {
        back,
        front,
        left,
        right,
        count
    };

    inline constexpr const char* kPlayerTextures[static_cast<std::size_t>(PlayerTextureNames::count)] =
    {
        "player/player_back.png",
        "player/player_front.png",
        "player/player_left.png",
        "player/player_right.png"
    };

    /// [ パーティクル ]
    inline constexpr const char* kParticleCircle = "particle/circle_256x256_white.dds";

    /// [ ローディング画面 ]
    inline constexpr const char* kLoading = "load/NOWLOADING.dds";

    // 数字
	inline constexpr const char* kUINumbers = "game/ui/number_x96y128.png";

    /// [ インゲーム ]
    namespace InGame
    {
        namespace GamePad
        {
            inline constexpr const char* kGuideShoot = "guide/shoot_gamepad.png";
            inline constexpr const char* kGuideSlow = "guide/slomo_gamepad.png";
            inline constexpr const char* kGuideExplosion = "guide/explosion_gamepad.png";
            inline constexpr const char* kGuidePause = "guide/pause_gamepad.png";
        }

        // ガイド画像
        inline constexpr const char* kGuideShoot = "guide/shoot.dds";
        inline constexpr const char* kGuideSlow = "guide/slomo.dds";
        inline constexpr const char* kGuideExplosion = "guide/explosion.dds";
        inline constexpr const char* kGuidePause = "guide/pause.dds";
        // レティクル
        inline constexpr const char* kReticle = "reticle.dds";
        // 爆発エンティティ
        inline constexpr const char* kExplosionRing = "game/player_explosion.dds";

        inline constexpr const char* kScoreUpRate2 = "game/x2.png";
        inline constexpr const char* kScoreUpRate3 = "game/x3.png";


        // UI
        inline constexpr const char* kProgressBar = "game/ui/progressBar.png";
        inline constexpr const char* kProgressBarFrame = "game/ui/progressBar_frame.png";
        inline constexpr const char* kProgressBarText = "game/ui/progressBar_text.png";
        inline constexpr const char* kTimelimitText = "game/ui/timelimit_text.png";
        inline constexpr const char* kMoveExplanationText = "game/ui/move_Explanation_UI.png";
        inline constexpr const char* kClearText = "game/ui/clear_text_UI.png";

        // マップタイル
        inline constexpr const char* kTestTile = "game/tile/Simple.png";
        inline constexpr const char* kTile = "game/tile/floor.png";


		enum class WallType
		{
			kAll,
			kLB,
			kLBT,
			kLRB,
			kLRT,
			kLT,
			kRB,
			kRBT,
			kRL,
			kRT,
			kTB,
            kB,
            kL,
            kR,
            kT,
            kW,
			WallTypeCount
		};

        // 壁
        inline constexpr const char* kWalls[static_cast<std::size_t>(WallType::WallTypeCount)] = {
            "game/walls/wall_all.png", // kAll
            "game/walls/wall_LB.png",  // kLB
            "game/walls/wall_LBT.png", // kLBT
            "game/walls/wall_LRB.png", // kLRB
            "game/walls/wall_LRT.png", // kLRT
            "game/walls/wall_LT.png",  // kLT
            "game/walls/wall_RB.png",  // kRB
            "game/walls/wall_RBT.png", // kRBT
            "game/walls/wall_RL.png",  // kRL
            "game/walls/wall_RT.png",  // kRT
            "game/walls/wall_TB.png",  // kTB
            "game/walls/wall_end_B.png",  // kB
            "game/walls/wall_end_L.png",  // kL
            "game/walls/wall_end_R.png",  // kR
            "game/walls/wall_end_T.png",  // kT
            "game/walls/wall.png",        // kW
        };

		// objects
		inline constexpr const char* kRouter = "game/objects/router.png";
		inline constexpr const char* kRepeater = "game/objects/repeater.png";
		inline constexpr const char* kPC = "game/objects/PC.png";
        inline constexpr const char* kAluminum = "game/objects/aluminum.png";

		enum class RotatingFloorType
		{
            kRight,
			kLeft,
			RotatingFloorTypeCount
		};
        inline constexpr const char* kRotatingFloors[static_cast<std::size_t>(RotatingFloorType::RotatingFloorTypeCount)] =
        { 
            "game/objects/rotating_floor_R.png", 
            "game/objects/rotating_floor_L.png" 
        };
    }


    /// [ ポーズメニュー ]
    namespace PauseMenu
    {
        inline constexpr const char* kBg = "game/pause/bg.dds";
        inline constexpr const char* kPaused = "game/pause/paused.dds";
        inline constexpr const char* kButtonResume = "game/pause/button_resume.dds";
        inline constexpr const char* kButtonRestart = "game/pause/button_retry.dds";
        inline constexpr const char* kButtonBackToTitle = "game/pause/button_back_to_title.dds";
    }
}

namespace Path::Audio
{
    /// [ BGM ]
    inline constexpr const char* kBgmTitle = "bgm_title.wav";
    inline constexpr const char* kBgmInGame = "bgm_ingame.wav";

    /// [ SE ]
    inline constexpr const char* kSeStartButton = "se_title_startbutton.wav";
    inline constexpr const char* kSePlayerShoot = "se_player_shoot.wav";
    inline constexpr const char* kSePlayerDeath = "se_player_death.wav";
    inline constexpr const char* kSePlayerSlowOn = "se_player_slow_on.wav";
    inline constexpr const char* kSePlayerSlowOff = "se_player_slow_off.wav";
    inline constexpr const char* kSeEnemyDeath = "se_enemy_death.wav";
    inline constexpr const char* kSeEnemyRusherAim = "se_enemyrusher_aim.wav";

    namespace UI
    {
        inline constexpr const char* kConfirm = "ui/confirm.wav";
        inline constexpr const char* kHover = "ui/hover.wav";
    }
}

namespace Path::Model
{
    inline constexpr const char* kGrid = "Grid_v3/Grid_v3.obj";
    inline constexpr const char* kParticlePlane = "particle/ParticlePlane.obj";
    inline constexpr const char* kPlayer = "Cube/Cube.obj";
    inline constexpr const char* kEnemy = "Cube/Cube.obj";
}

namespace Path::ParticleEmitter
{
    inline constexpr const char* kSaveDir = "resources/json/particles/";
    inline constexpr const char* kPlayerConstantTrail = "Player_Constant_Trail.json";
    inline constexpr const char* kGameOverExplosion = "Scene_GameOver_Explosion.json";
    inline constexpr const char* kGameClearExplosionOrange = "Scene_GameClear_Explosion_Orange.json";
    inline constexpr const char* kGameClearExplosionYellow = "Scene_GameClear_Explosion_Yellow.json";
    inline constexpr const char* kGameClearExplosionBlue = "Scene_GameClear_Explosion_Blue.json";
    inline constexpr const char* kEnemyNormalDeathExplosion = "EnemyNormal_Death_Explosion.json";
    inline constexpr const char* kEnemyNormalDeathSpark = "EnemyNormal_Death_Spark.json";
}

namespace Path::Json
{
    inline constexpr const char* kPopTimeTable = "PopTimeTable.json";
    inline constexpr const char* kScoreReviewerThresholds = "ScoreReviewerThresholds.json";
    inline constexpr const char* kMapDir = "maps/";
}

namespace Path {
namespace Image {
namespace InGame {

inline constexpr std::string_view kWall = "images/in_game/wall.png";

} // namespace InGame
} // namespace Image
} // namespace Path