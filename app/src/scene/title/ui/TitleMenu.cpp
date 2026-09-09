#include "TitleMenu.h"
#include <config/ResourcePath.h>



void TitleMenu::InitializeSprites()
{
    for (uint32_t i = 0; i < pSprites_.size(); ++i)
    {
        pSprites_[i] = std::make_unique<Sprite>();
        pSprites_[i]->Initialize(Path::Image::Title::kMenuItemStart);
    }
}
