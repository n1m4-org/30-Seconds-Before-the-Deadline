#pragma once
#include <memory>
#include <drawable/sprite/Sprite.h>
#include <config/ResourcePath.h>
#include <array>
#include <Features/Animation/AnimationTimeline.hpp>

class PlayerPopupAnimation
{
public:
    void Initialize();
    void Update();
    void Draw1F();
    void ImGui();

private:
    void InitializeSprite();
    void InitializeTextureHandle();
    void LotteryPlayerTexture();

    std::unique_ptr<DebugEntry<PlayerPopupAnimation>> pDebugEntry_ = nullptr;

    AnimationTimeline<Vector2> timelinePosition_;
    std::unique_ptr<Sprite> pSpritePlayer_ = nullptr;
    std::array<D3D12_GPU_DESCRIPTOR_HANDLE, static_cast<size_t>(Path::Image::PlayerTextureNames::count)> playerTextureHandles_;
};