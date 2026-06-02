#include "Game.h"
#include "../Hooks/D3DHook.h"
#include "../Rhythm/RhythmSystem.h"
#include <cstdlib>
#include <ctime>

namespace Game
{
    static std::string gCurrentBeatmapId;

    void Initialize()
    {
		srand(static_cast<unsigned>(time(nullptr)));

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
