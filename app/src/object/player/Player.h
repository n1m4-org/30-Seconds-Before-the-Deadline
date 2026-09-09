#pragma once
#include <object/baseObject2d/BaseObject2d.h>
#include <Features/Input/Input.h>
#include <logic/mapCollision/MapCollision.h>
#include <vector>
#include <memory>

class Player : public BaseObject2d
{
public:
	Player();
	~Player() override;

	void Initialize() override;
	void Update() override;
	void Draw() override;

	// WASD入力と押し出し制御 (移動成功時に true を返す)
	bool HandleInput(Input* pInput, MapCollision& mapCollision,
	                 const std::vector<std::vector<int>>& currentMap,
	                 const std::vector<std::unique_ptr<BaseObject2d>>& objects);

	// 向きに応じたテクスチャの強制更新（回転角リセット含む）
	void ForceUpdateTexture();

	// 長押し状態のリセット
	void ResetHoldState()
	{
		holdDir_ = { 0, 0 };
		holdTimer_ = 0.0f;
		repeatTimer_ = 0.0f;
	}

private:
	void UpdateSpriteTextureBasedOnAngle();

	void SetSpriteTexture(size_t textureIndex);

	Vector2Int beforeAngle_ = { 0, -1 }; // 前回の角度を保持

	int anmationFrame_ = 0; // アニメーションフレームのカウンタ

	Vector2 originalSpriteSize_ = { 100.0f, 100.0f }; // 元のスプライトサイズ

	// 長押し（オートリピート）用 (DeltaTime秒数ベース)
	Vector2Int holdDir_ = { 0, 0 };
	float holdTimer_ = 0.0f;
	float repeatTimer_ = 0.0f;
	static constexpr float kInitialRepeatDelay = 0.35f; // 長押し開始遅延 (0.35秒)
	static constexpr float kRepeatInterval = 0.22f;      // 連続移動間隔 (0.22秒)
};

