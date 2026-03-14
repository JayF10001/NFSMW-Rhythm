#include "GameState.h"

static GameState gState = GameState::Inactive;

namespace Game
{
    void SetState(GameState state)
    {
        gState = state;
    }

    GameState GetState()
    {
        return gState;
    }
}
