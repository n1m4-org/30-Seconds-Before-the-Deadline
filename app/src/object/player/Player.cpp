#include "Player.h"
#include <config/ResourcePath.h>
#include <dinput.h>
#include <Features/DeltaTimeManager/DeltaTimeManager.h>

Player::Player()
{
	objectType_ = ObjectType2d::kPlayer;
	isDynamic_ = true;
}

Player::~Player()
{
}

void Player::Initialize()
{
	objectType_ = ObjectType2d::kPlayer;
	isDynamic_ = true;
	angle_ = Vector2Int{ 0, 1 };
	beforeAngle_ = angle_;

	pSprite_ = std::make_unique<Sprite>();
	originalSpriteSize_ = pSprite_->GetSize();
	SetSpriteTexture(static_cast<size_t>(Path::Image::PlayerTextureNames::front));
}

bool Player::HandleInput(Input* pInput, MapCollision& mapCollision,
                         const std::vector<std::vector<int>>& currentMap,
                         const std::vector<std::unique_ptr<BaseObject2d>>& objects)
{
	if (!pInput)
	{
		return false;
	}

	// 現在押されている方向を取得（PushKeyでホールド判定。WASD + 矢印キー）
	Vector2Int curPushDir = { 0, 0 };
	if (pInput->PushKey(DIK_W) || pInput->PushKeyC('W') || pInput->PushKeyC('w') || pInput->PushKey(DIK_UP))
	{
		curPushDir = { 0, -1 };
	}
	else if (pInput->PushKey(DIK_S) || pInput->PushKeyC('S') || pInput->PushKeyC('s') || pInput->PushKey(DIK_DOWN))
	{
		curPushDir = { 0, 1 };
	}
	else if (pInput->PushKey(DIK_A) || pInput->PushKeyC('A') || pInput->PushKeyC('a') || pInput->PushKey(DIK_LEFT))
	{
		curPushDir = { -1, 0 };
	}
	else if (pInput->PushKey(DIK_D) || pInput->PushKeyC('D') || pInput->PushKeyC('d') || pInput->PushKey(DIK_RIGHT))
	{
		curPushDir = { 1, 0 };
	}

	bool shouldMove = false;
	Vector2Int moveDir = { 0, 0 };

	float dt = 1.0f / 60.0f;
	try
	{
		dt = DeltaTimeManager::GetInstance()->GetDeltaTime(static_cast<uint32_t>(DeltaTimeChannelReserved::Game));
	}
	catch (...)
	{
		dt = 1.0f / 60.0f;
	}

	if (curPushDir.x != 0 || curPushDir.y != 0)
	{
		if (curPushDir != holdDir_)
		{
			// 新しい方向が押された：即座に1歩移動し、長押しタイマー開始
			holdDir_ = curPushDir;
			holdTimer_ = 0.0f;
			repeatTimer_ = 0.0f;
			shouldMove = true;
			moveDir = curPushDir;
		}
		else
		{
			// 同じ方向を押し続けている
			holdTimer_ += dt;
			if (holdTimer_ >= kInitialRepeatDelay)
			{
				repeatTimer_ += dt;
				if (repeatTimer_ >= kRepeatInterval)
				{
					repeatTimer_ -= kRepeatInterval;
					shouldMove = true;
					moveDir = curPushDir;
				}
			}
		}
	}
	else
	{
		// 何も押されていない：リセット
		holdDir_ = { 0, 0 };
		holdTimer_ = 0.0f;
		repeatTimer_ = 0.0f;
	}

	if (shouldMove)
	{
		// 連続移動でまだ前回の補間が残っている場合は即座に目標位置へスナップして次へ進む
		if (isMoving_)
		{
			currentRenderPos_ = targetRenderPos_;
			isMoving_ = false;
		}

		angle_ = moveDir;
		if (mapCollision.TryMove(currentMap, objects, *this, moveDir))
		{
			anmationFrame_ = (anmationFrame_ == 0) ? 1 : 0; // アニメーションフレーム切り替え
			Vector2 spriteSize = pSprite_->GetSize();
			pSprite_->SetTextureLeftTop({ static_cast<float>(anmationFrame_) * 128.0f, 0.0f });
			return true;
		}
	}

	return false;
}

void Player::ForceUpdateTexture()
{
	if (pSprite_)
	{
		pSprite_->SetRotation(0.0f); // プレイヤーはテクスチャ画像で向きを表すためスプライトは回転させない
	}

	if (angle_ == Vector2Int{ 0, -1 }) // 上
	{
		SetSpriteTexture(static_cast<size_t>(Path::Image::PlayerTextureNames::back));
	}
	else if (angle_ == Vector2Int{ 0, 1 }) // 下
	{
		SetSpriteTexture(static_cast<size_t>(Path::Image::PlayerTextureNames::front));
	}
	else if (angle_ == Vector2Int{ -1, 0 }) // 左
	{
		SetSpriteTexture(static_cast<size_t>(Path::Image::PlayerTextureNames::left));
	}
	else if (angle_ == Vector2Int{ 1, 0 }) // 右
	{
		SetSpriteTexture(static_cast<size_t>(Path::Image::PlayerTextureNames::right));
	}
	beforeAngle_ = angle_;
}

void Player::UpdateSpriteTextureBasedOnAngle()
{
	if (beforeAngle_ != angle_)
	{
		ForceUpdateTexture();
	}
}

void Player::SetSpriteTexture(size_t textureIndex)
{
	pSprite_->Initialize(Path::Image::kPlayerTextures[textureIndex]);
	Vector2 spriteSize = pSprite_->GetSize();
	pSprite_->SetTextureSize({ spriteSize.x / 2.0f, spriteSize.y });
	pSprite_->SetTextureLeftTop({ static_cast<float>(anmationFrame_) * spriteSize.x / 2.0f, 0.0f });
}

void Player::Update()
{
	BaseObject2d::Update();
	UpdateSpriteTextureBasedOnAngle();
	beforeAngle_ = angle_;
}

void Player::Draw()
{
	BaseObject2d::Draw();
}

