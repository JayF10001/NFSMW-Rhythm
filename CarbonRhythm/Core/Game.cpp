#include "Game.h"
#include "../Hooks/D3DHook.h"
#include "../Rhythm/RhythmSystem.h"

namespace Game
{
    static std::string gCurrentBeatmapId;

    void Initialize()
    {
        Rhythm::Initialize();
        D3DHook::Initialize();
    }

    void Game::SetCurrentBeatmapId(const std::string& id)
    {
        gCurrentBeatmapId = id;
    }

    const std::string& Game::GetCurrentBeatmapId()
    {
        return gCurrentBeatmapId;
    }

}
