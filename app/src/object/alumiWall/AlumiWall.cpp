#include "AlumiWall.h"
#include <config/ResourcePath.h>

AlumiWall::AlumiWall()
{
	objectType_ = ObjectType2d::kAlumiWall;
	isDynamic_ = true;
}

AlumiWall::~AlumiWall()
{
}

void AlumiWall::Initialize()
{
	objectType_ = ObjectType2d::kAlumiWall;
	isDynamic_ = true;

	pSprite_ = std::make_unique<Sprite>();
	pSprite_->Initialize(Path::Image::InGame::kAluminum);
}

void AlumiWall::Update()
{
	BaseObject2d::Update();
}

void AlumiWall::Draw()
{
	BaseObject2d::Draw();
}

