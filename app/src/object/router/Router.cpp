#include "Router.h"
#include <config/ResourcePath.h>

Router::Router()
{
	objectType_ = ObjectType2d::kRouter;
	isDynamic_ = false;
}

Router::~Router()
{
}

void Router::Initialize()
{
	objectType_ = ObjectType2d::kRouter;
	isDynamic_ = false;

	pSprite_ = std::make_unique<Sprite>();
	pSprite_->Initialize(Path::Image::InGame::kRouter);
	//pSprite_->SetColor({ 1.0f, 0.8f, 0.2f, 1.0f }); // 仮置き：黄色/橙色
}

void Router::Update()
{
	BaseObject2d::Update();
	ApplyRotationToSprite();
}

void Router::Draw()
{
	BaseObject2d::Draw();
}

