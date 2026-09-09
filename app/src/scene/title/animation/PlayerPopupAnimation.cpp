#include "PlayerPopupAnimation.h"
#include <Core/DirectX12/TextureManager.h>
#include <Features/RandomGenerator/RandomGenerator.h>


void PlayerPopupAnimation::Initialize()
{
    this->InitializeSprite();
    this->InitializeTextureHandle();

#ifdef _DEBUG
    reg_ = AnimationEditor::GetInstance()->Register("PlayerPopup", timelinePosition_);
#endif // _DEBUG
}

void PlayerPopupAnimation::Update()
{
    auto pos = timelinePosition_.Update();
    pSpritePlayer_->SetPosition(pos);
    pSpritePlayer_->Update();
}

void PlayerPopupAnimation::Draw1F()
{
    pSpritePlayer_->Draw1F();
}

void PlayerPopupAnimation::InitializeSprite()
{
    pSpritePlayer_ = std::make_unique<Sprite>();
    pSpritePlayer_->Initialize(Path::Image::kPlayerTextures[0]);
    pSpritePlayer_->SetAnchorPoint({ 0.5f, 0.5f });
    pSpritePlayer_->SetTextureSize({ 128.0f, 128.0f });
}

void PlayerPopupAnimation::InitializeTextureHandle()
{
    for (uint32_t i = 0; i < playerTextureHandles_.size(); ++i)
    {
        auto& handle = playerTextureHandles_[i];
        TextureManager::GetInstance()->LoadTexture(Path::Image::kPlayerTextures[i]);
        handle = TextureManager::GetInstance()->GetSrvHandleGPU(Path::Image::kPlayerTextures[i]);
    }
}

void PlayerPopupAnimation::LotteryPlayerTexture()
{
    auto pRand = RandomGenerator::GetInstance();
    const float maxRand = static_cast<float>(playerTextureHandles_.size() - 1);
    auto numRand = pRand->Generate<float>(0.0f, maxRand);
    pSpritePlayer_->SetTextureHandle(playerTextureHandles_[static_cast<uint32_t>(numRand)]);
}
