#include "RhythmSystem.h"
#include <Windows.h>
#include <vector>
#include <cmath>
#include <chrono>
#include "Core/GameTime.h"
#include "Input.h"
#include "Audio/AudioSystem.h"

extern uint64_t bGetTicker();
extern float& TicksToMilliseconds;

namespace Rhythm
{
    static Judgement lastJudgement = Judgement::None;
    static uint64_t lastTick = 0;
    static float timer = 0.0f;
    static int currentBeat = -1;
    static float bpm = 140.0f;

    static double perfectWindow = 0.045;  // 30ms
    static double goodWindow = 0.075;  // 60ms
    static double badWindow = 0.115;  // 100ms

    static int multiplier = 1;
    static const int maxMultiplier = 20;

    static bool windowActive = false;

    static int score = 0;
    static int combo = 0;
    static bool lastResultSuccess = false;
    static float resultFlashTimer = 0.0f;

    struct Note
    {
        double time;
        int keyMask;   // bitmask tombol
        bool hit;
    };



    static std::vector<Note> notes;
    static size_t nextNoteIndex = 0;
    static double audioOffset = -0.05;

    void Initialize()
    {
        timer = 0.0f;
        currentBeat = -1;
        windowActive = false;
        score = 0;
        combo = 0;
        lastResultSuccess = false;
        resultFlashTimer = 0.0f;
        lastTick = 0;

        notes.clear();
        nextNoteIndex = 0;
    }



    bool IsInBeatWindow()
    {
        float secondsPerBeat = 60.0f / bpm;
        float beatTime = fmod(timer, secondsPerBeat);

        return (beatTime < badWindow || beatTime > secondsPerBeat - badWindow);
    }


    float GetBeatPhase()
    {
        float secondsPerBeat = 60.0f / bpm;
        float beatTime = fmod(timer, secondsPerBeat);
        return beatTime / secondsPerBeat;
    }

    void Update()
    {
        timer = Audio::GetPositionSeconds() + audioOffset;
        float dt = 0.0f; // sementara biar compile


        float secondsPerBeat = 60.0f / bpm;
        int beat = (int)(timer / secondsPerBeat);

        while (nextNoteIndex < notes.size())
        {
            Note& note = notes[nextNoteIndex];

            if (!note.hit && timer > note.time + badWindow)
            {
                combo = 0;
                lastResultSuccess = false;
                lastJudgement = Judgement::Miss;
                resultFlashTimer = 0.2f;

                note.hit = true;
                nextNoteIndex++;
            }
            else
                break;
        }



        bool inWindow = IsInBeatWindow();

        if (inWindow && !windowActive)
        {
            windowActive = true;
            OutputDebugStringA("WINDOW OPEN\n");
        }

        if (!inWindow && windowActive)
        {
            windowActive = false;
            OutputDebugStringA("WINDOW CLOSE\n");
        }

        int pressedMask = Input::GetPressedMask();

        if (pressedMask != 0 && nextNoteIndex < notes.size())
        {
            Note& note = notes[nextNoteIndex];

            double diff = timer - note.time;

            if (diff >= -badWindow && diff <= badWindow)
            {
                if ((pressedMask & note.keyMask) == note.keyMask)
                {
                    if (fabs(diff) <= perfectWindow)
                    {
                        combo++;
                        UpdateMultiplier();
                        score += 300 * multiplier;
                        lastJudgement = Judgement::Perfect;
                    }
                    else if (fabs(diff) <= goodWindow)
                    {
                        combo++;
                        UpdateMultiplier();
                        score += 150 * multiplier;
                        lastJudgement = Judgement::Good;
                    }
                    else
                    {
                        score += 50;
                        combo = 0;
                        multiplier = 1;
                        lastJudgement = Judgement::Bad;
                    }

                    lastResultSuccess = true;
                    resultFlashTimer = 0.2f;

                    note.hit = true;
                    nextNoteIndex++;
                }
                else
                {
                    combo = 0;
                    multiplier = 1;
                    lastResultSuccess = false;
                    lastJudgement = Judgement::Miss;
                    resultFlashTimer = 0.2f;

                    note.hit = true;
                    nextNoteIndex++;
                }
            }
        }




        if (resultFlashTimer > 0.0f)
            resultFlashTimer -= dt;
    }

    int GetScore() { return score; }
    int GetCombo() { return combo; }
    bool IsFlashing() { return resultFlashTimer > 0.0f; }
    bool WasLastSuccess() { return lastResultSuccess; }

    bool IsWindowActive()
    {
        return windowActive;
    }


    float GetTime()
    {
        return timer;
    }

    bool HasNextNote()
    {
        return nextNoteIndex < notes.size();
    }

    double GetNextNoteTime()
    {
        if (nextNoteIndex < notes.size())
            return notes[nextNoteIndex].time;
        return -1.0;
    }


    int GetCurrentBeat()
    {
        return currentBeat;
    }

    double GetSecondsPerBeat()
    {
        return 60.0 / bpm;
    }

    Judgement GetLastJudgement()
    {
        return lastJudgement;
    }
    size_t GetCurrentNoteIndex()
    {
        return nextNoteIndex;
    }

    bool IsNoteValid(size_t index)
    {
        return index < notes.size();
    }

    double GetNoteTime(size_t index)
    {
        if (index < notes.size())
            return notes[index].time;
        return -1.0;
    }

    int GetNoteKeyMask(size_t index)
    {
        if (index < notes.size())
            return notes[index].keyMask;
        return 0;
    }

    void ClearNotes()
    {
        notes.clear();
        nextNoteIndex = 0;
    }

    void AddNote(double time, int mask)
    {
        notes.push_back({ time, mask, false });
    }

    void SetBPM(double value)
    {
        bpm = (float)value;
    }

    void SetOffset(double value)
    {
        audioOffset = value;
    }

    void SetJudgementWindows(double perfect, double good, double bad)
    {
        perfectWindow = perfect;
        goodWindow = good;
        badWindow = bad;
    }

    static void UpdateMultiplier()
    {
        multiplier = 1 + (combo / 10);

        if (multiplier > maxMultiplier)
            multiplier = maxMultiplier;
    }

    int GetMultiplier() { return multiplier; }

}
