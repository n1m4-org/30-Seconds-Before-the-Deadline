#pragma once
#include <object/baseObject2d/BaseObject2d.h>

class AlumiWall : public BaseObject2d
{
public:
	AlumiWall();
	~AlumiWall() override;

	void Initialize() override;
	void Update() override;
	void Draw() override;

	// アルミ壁は向きによる回転を行わない（常に正立）
	void ApplyRotationToSprite() override { ResetRotation(); }
};
