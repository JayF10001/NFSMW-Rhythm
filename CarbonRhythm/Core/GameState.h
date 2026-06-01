#pragma once

enum class GameState
{
    Inactive,   // default
    Menu,
    Playing,
    Recording
};


namespace Game
{
    void SetState(GameState state);
    GameState GetState();
}
