#include "RotatingFloor.h"
#include <config/ResourcePath.h>

RotatingFloor::RotatingFloor()
{
	objectType_ = ObjectType2d::kRotatingFloor;
	isDynamic_ = false; // 床オブジェクトのため非動的（プレイヤーや他オブジェクトが上に乗れる）
}

RotatingFloor::~RotatingFloor()
{
}

void RotatingFloor::Initialize()
{
	objectType_ = ObjectType2d::kRotatingFloor;
	isDynamic_ = false;

	pSprite_ = std::make_unique<Sprite>();
	pSprite_->Initialize(Path::Image::InGame::kRotatingFloors[static_cast<std::size_t>(rotatingFloorType_)]);
}

void RotatingFloor::SetRotatingFloorType(RotatingFloorType type)
{
	rotatingFloorType_ = type;
	if (pSprite_)
	{
		pSprite_->Initialize(Path::Image::InGame::kRotatingFloors[static_cast<std::size_t>(rotatingFloorType_)]);
	}
}

void RotatingFloor::Update()
{
	BaseObject2d::Update();
	ApplyRotationToSprite();
}

void RotatingFloor::Draw()
{
	BaseObject2d::Draw();
}

void RotatingFloor::CheckAndRotateRepeater(const std::vector<std::unique_ptr<BaseObject2d>>& objects)
{
	if (!isActive_)
	{
		return;
	}

	BaseObject2d* currentRepeaterOnFloor = nullptr;

	for (const auto& obj : objects)
	{
		if (obj && obj->GetObjectType() == ObjectType2d::kRepeater)
		{
			if (obj->GetPosition() == position_)
			{
				currentRepeaterOnFloor = obj.get();
				break;
			}
		}
	}

	if (currentRepeaterOnFloor)
	{
		// 新しくこの回転床の上に乗った場合のみ、向きを1度だけ90度変更して消滅する
		if (pOccupyingRepeater_ != currentRepeaterOnFloor)
		{
			Vector2Int curAngle = currentRepeaterOnFloor->GetAngle();
			Vector2Int newAngle = curAngle;

			if (rotatingFloorType_ == RotatingFloorType::kRight)
			{
				// 右回転 (時計回り): (x, y) -> (-y, x)
				newAngle = { -curAngle.y, curAngle.x };
			}
			else if (rotatingFloorType_ == RotatingFloorType::kLeft)
			{
				// 左回転 (反時計回り): (x, y) -> (y, -x)
				newAngle = { curAngle.y, -curAngle.x };
			}

			currentRepeaterOnFloor->SetAngle(newAngle);
			pOccupyingRepeater_ = currentRepeaterOnFloor;

			// 一度乗って回転したら、回転床自体が消滅する
			isActive_ = false;
		}
	}
	else
	{
		// 中継器が離れた場合は占有状態を解除
		pOccupyingRepeater_ = nullptr;
	}
}
