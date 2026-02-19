#pragma once

namespace Rhythm
{
    enum class Judgement
    {
        None,
        Perfect,
        Good,
        Bad,
        Miss
    };

    float GetBeatPhase();
    void Initialize();
    void Update();
    float GetTime();
    bool IsInBeatWindow();
    bool IsWindowActive();
    int GetScore();
    int GetCombo();
    bool IsFlashing();
    bool WasLastSuccess();
    bool HasNextNote();
    double GetSecondsPerBeat();
    int GetCurrentBeat();
    double GetNextNoteTime();
    Judgement GetLastJudgement();
    size_t GetCurrentNoteIndex();
    bool IsNoteValid(size_t index);
    double GetNoteTime(size_t index);
    int GetNoteKeyMask(size_t index);
    void ClearNotes();
    void AddNote(double time, int mask);
    void SetBPM(double value);
    void SetOffset(double value);
    void SetJudgementWindows(double perfect, double good, double bad);
}
