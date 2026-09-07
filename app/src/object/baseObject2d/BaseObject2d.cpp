#include "BaseObject2d.h"
#include <algorithm>
#include <cmath>
#include <Features/DeltaTimeManager/DeltaTimeManager.h>

BaseObject2d::BaseObject2d()
{
}

BaseObject2d::~BaseObject2d()
{
}

void BaseObject2d::SetPosition(const Vector2Int& position)
{
	position_ = position;
	targetPosition_ = position;
	isMoving_ = false;
	moveProgress_ = 0.0f;
}

void BaseObject2d::Move(const Vector2Int& delta)
{
	position_ = position_ + delta;
	targetPosition_ = position_;
	isMoving_ = true;
	moveProgress_ = 0.0f;
}


void BaseObject2d::UpdateSpritePosition(float tileSize, const Vector2& mapOffset)
{
	Vector2 calcPos = {
		tileSize / 2.0f + tileSize * position_.x + mapOffset.x,
		tileSize / 2.0f + tileSize * position_.y + mapOffset.y
	};

	if (!isMoving_)
	{
		currentRenderPos_ = calcPos;
		targetRenderPos_ = calcPos;
	}
	else
	{
		// 移動開始時のスタート位置から新しい目標位置へ
		targetRenderPos_ = calcPos;
	}

	if (pSprite_)
	{
		pSprite_->SetAnchorPoint({ 0.5f, 0.5f });
		pSprite_->SetPosition(currentRenderPos_);
		pSprite_->SetSize(size_);
	}


}

void BaseObject2d::Update()
{
	if (!isActive_)
	{
		return;
	}

	if (isMoving_)
	{
		float dt = 1.0f / 60.0f;
		try
		{
			dt = DeltaTimeManager::GetInstance()->GetDeltaTime(static_cast<uint32_t>(DeltaTimeChannelReserved::Game));
		}
		catch (...)
		{
			dt = 1.0f / 60.0f;
		}

		// 60FPS時の補間係数 0.25f と同等となる DeltaTime 依存の指数減衰補間
		float blend = 1.0f - std::pow(0.75f, dt * 60.0f);
		blend = std::clamp(blend, 0.0f, 1.0f);

		currentRenderPos_.x += (targetRenderPos_.x - currentRenderPos_.x) * blend;
		currentRenderPos_.y += (targetRenderPos_.y - currentRenderPos_.y) * blend;

		float distSq = (targetRenderPos_.x - currentRenderPos_.x) * (targetRenderPos_.x - currentRenderPos_.x) +
		               (targetRenderPos_.y - currentRenderPos_.y) * (targetRenderPos_.y - currentRenderPos_.y);

		if (distSq < 0.1f)
		{
			currentRenderPos_ = targetRenderPos_;
			isMoving_ = false;
		}

		if (pSprite_)
		{
			pSprite_->SetPosition(currentRenderPos_);
		}
	}

	if (pSprite_)
	{
		pSprite_->Update();
	}
}

void BaseObject2d::Draw()
{
	if (!isActive_)
	{
		return;
	}

	if (pSprite_)
	{
		pSprite_->Draw1F();
	}
}
