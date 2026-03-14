#include "RhythmSystem.h"
#include <Windows.h>
#include <vector>
#include <cmath>
#include "Core/GameTime.h"
#include "Input.h"
#include "Audio/AudioSystem.h"
#include <Core/GameState.h>
#include <Car/DanceController.h>
#include "ScoreSystem.h"
#include <Core/Game.h>

namespace Rhythm
{
    static Judgement lastJudgement = Judgement::None;
    static uint64_t lastCounter = 0;
    static float timer = 0.0f;
    static int currentBeat = -1;
    static double bpm = 140.0;

    static double perfectWindow = 0.045;
    static double goodWindow = 0.075;
    static double badWindow = 0.115;

    static int multiplier = 1;
    static const int maxMultiplier = 20;

    static bool windowActive = false;

    static int score = 0;
    static int combo = 0;
    static bool lastResultSuccess = false;
    static float resultFlashTimer = 0.0f;

    static bool songFinished = false;
    static float finishTimer = 0.0f;

    static int totalNotes = 0;
    static int hitPerfect = 0;
    static int hitGood = 0;
    static int hitBad = 0;
    static int hitMiss = 0;

    static int maxCombo = 0;

    static float gDeltaTime = 0.0f;

    static int lastHitMask = 0;

    static int lastHitNoteIndex = -1;

    struct Note
    {
        double time;
        int keyMask;
        bool hit;
        float hitVisualTimer;
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
        lastCounter = 0;

        notes.clear();
        nextNoteIndex = 0;

        songFinished = false;
        finishTimer = 0.0f;

        totalNotes = 0;
        hitPerfect = 0;
        hitGood = 0;
        hitBad = 0;
        hitMiss = 0;
        maxCombo = 0;
        multiplier = 1;
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
        float dt = GameTime::DeltaSeconds(lastCounter);
        gDeltaTime = dt;

        float secondsPerBeat = 60.0f / bpm;
        int beat = (int)(timer / secondsPerBeat);

        for (auto& note : notes)
        {
            if (note.hitVisualTimer > 0.0f)
            {
                note.hitVisualTimer -= dt;
                if (note.hitVisualTimer < 0.0f)
                    note.hitVisualTimer = 0.0f;
            }
        }

        while (nextNoteIndex < notes.size())
        {
            Note& note = notes[nextNoteIndex];

            if (!note.hit && timer > note.time + badWindow)
            {
                combo = 0;
                lastResultSuccess = false;
                lastJudgement = Judgement::Miss;
                resultFlashTimer = 0.2f;

                lastHitNoteIndex = nextNoteIndex;

                note.hit = true;
                note.hitVisualTimer = 0.12f;
                nextNoteIndex++;
                multiplier = 1;
                hitMiss++;
                totalNotes++;
            }
            else
                break;
        }



        bool inWindow = IsInBeatWindow();

        if (inWindow && !windowActive)
        {
            windowActive = true;
        }

        if (!inWindow && windowActive)
        {
            windowActive = false;
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
                        hitPerfect++;
                        totalNotes++;
                        if (combo > maxCombo)
                            maxCombo = combo;
                    }
                    else if (fabs(diff) <= goodWindow)
                    {
                        combo++;
                        UpdateMultiplier();
                        score += 150 * multiplier;
                        lastJudgement = Judgement::Good;
                        hitGood++;
                        totalNotes++;
                        if (combo > maxCombo)
                            maxCombo = combo;
                    }
                    else
                    {
                        score += 50;
                        combo = 0;
                        multiplier = 1;
                        lastJudgement = Judgement::Bad;
                        hitBad++;
                        totalNotes++;
                    }

                    lastResultSuccess = true;
                    resultFlashTimer = 0.2f;

                    note.hit = true;
                    lastHitNoteIndex = nextNoteIndex;

                    note.hit = true;
                    note.hitVisualTimer = 0.12f;

                    nextNoteIndex++;
                    lastHitMask = note.keyMask;

                    lastHitMask = note.keyMask;
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
                    hitMiss++;
                    totalNotes++;
                }
            }
        }




        if (resultFlashTimer > 0.0f)
            resultFlashTimer -= dt;

        // check if beatmap done
        if (!songFinished && nextNoteIndex >= notes.size())
        {
            songFinished = true;
            finishTimer = 3.0f; // 3 seconds delay
        }

        if (songFinished)
        {
            finishTimer -= dt;

            if (finishTimer <= 0.0f)
            {
                songFinished = false;

                Audio::Stop();
                Dance::SetEnabled(false);

                double accuracy = GetAccuracy();

                Score::Save(
                    Game::GetCurrentBeatmapId(),
                    GetScore(),
                    maxCombo,
                    accuracy
                );

                Game::SetState(GameState::Menu);
            }
        }

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

    double GetAccuracy()
    {
        if (totalNotes == 0) return 0.0;

        double weighted =
            hitPerfect * 1.0 +
            hitGood * 0.7 +
            hitBad * 0.4;

        return (weighted / totalNotes) * 100.0;
    }

    float GetDeltaTime()
    {
        return gDeltaTime;
    }

    int GetLastHitMask() { return lastHitMask; }

    int GetLastHitNoteIndex()
    {
        return lastHitNoteIndex;
    }

    bool IsNoteHit(size_t index)
    {
        if (index < notes.size())
            return notes[index].hit;
        return false;
    }

    float GetNoteHitVisualTimer(size_t index)
    {
        if (index < notes.size())
            return notes[index].hitVisualTimer;
        return 0.0f;
    }
}
