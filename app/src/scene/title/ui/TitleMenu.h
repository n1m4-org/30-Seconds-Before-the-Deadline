#pragma once
#include <Utility/Layout/FlexContainer.h>
#include <Utility/Layout/FlexTypes.h>
#include <array>
#include <drawable/sprite/Sprite.h>
#include <memory>

class TitleMenu
{
public:

private:
    void InitializeSprites();

    enum class MenuItem
    {
        StartGame,
        Options,
        Exit,
        COUNT
    };

    static constexpr size_t kMenuItemCount = static_cast<size_t>(MenuItem::COUNT);

    FlexContainer fc_ = {};
    std::array<FlexItem, kMenuItemCount> flexItems_ = {};
    std::array<std::unique_ptr<Sprite>, kMenuItemCount> pSprites_ = {};
};