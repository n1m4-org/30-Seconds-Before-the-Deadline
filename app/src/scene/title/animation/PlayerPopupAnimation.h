#pragma once
#include <memory>
#include <drawable/sprite/Sprite.h>
#include <config/ResourcePath.h>
#include <array>
#include <Features/Animation/AnimationTimeline.hpp>
#include <Features/Animation/AnimationEditor.hpp>

class PlayerPopupAnimation
{
public:
    void Initialize();
    void Update();
    void Draw1F();

private:
    void InitializeSprite();
    void InitializeTextureHandle();
    void LotteryPlayerTexture();

#ifdef _DEBUG
    AnimationEditor::Registration reg_;
#endif // _DEBUG
    AnimationTimeline<Vector2> timelinePosition_;
    std::unique_ptr<Sprite> pSpritePlayer_ = nullptr;
    std::array<D3D12_GPU_DESCRIPTOR_HANDLE, static_cast<size_t>(Path::Image::PlayerTextureNames::count)> playerTextureHandles_;
};