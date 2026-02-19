#include "Game.h"
#include "../Hooks/D3DHook.h"
#include "../Rhythm/RhythmSystem.h"

namespace Game
{
    void Initialize()
    {
        Rhythm::Initialize();
        D3DHook::Initialize();
    }
}
