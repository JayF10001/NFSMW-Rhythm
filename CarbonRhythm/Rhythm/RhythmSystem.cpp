#include "RhythmSystem.h"
#include <Windows.h>
#include <vector>
#include <cmath>
#include "Core/GameTime.h"
#include "Input.h"
#include "Audio/AudioSystem.h"
#include <Core/GameState.h>
#include <Car/DanceController.h>
#include <Core/Config.h>
#include "ScoreSystem.h"
#include <Core/Game.h>

namespace Rhythm
{
    static Judgement lastJudgement = Judgement::None;
    static uint64_t lastCounter = 0;
    static float timer = 0.0f;
    static int currentBeat = -1;
    static double bpm = 140.0;
	static std::vector<BPMChange> bpmChanges;
	static size_t currentBpmIndex = 0;
	static double totalBeatTime = 0.0; // Cumulated time in Beats
	static double lastUpdateTime = 0.0;
	static double lastBPMChangeTime = 0.0;   // Time in seconds from last change of BPM

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

	static bool g_autoplay = false;  // Auto-play Support

    struct Note
    {
        double time;
		double beatTime;
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
		bpmChanges.clear();
		currentBpmIndex = 0;
		totalBeatTime = 0.0;
		lastBPMChangeTime = 0.0;
		bpm = 140.0; // Set as Fallback BPM
    }

	void UpdateBPM(double currentTime)
	{
		while (currentBpmIndex < bpmChanges.size())
		{
			BPMChange& nextChange = bpmChanges[currentBpmIndex];
			if (currentTime >= nextChange.time)
			{
				// Calculate time since previous change (or start)
				double prevTime = (currentBpmIndex == 0) ? 0.0 : bpmChanges[currentBpmIndex - 1].time;
				double prevBPM  = (currentBpmIndex == 0) ? bpm : bpmChanges[currentBpmIndex - 1].bpm;
				double deltaSeconds = nextChange.time - prevTime;
				totalBeatTime += deltaSeconds * (prevBPM / 60.0);

				// Apply new BPM and remember the change time
				bpm = nextChange.bpm;
				lastBPMChangeTime = nextChange.time;
				currentBpmIndex++;
			}
			else
				break;
		}
		if (Config::GetRecorder().Dance.SyncJellyToCurrentBPM)
		{
			Dance::SetJellyBPM((float)bpm);
		}
	}


    bool IsInBeatWindow()
    {
        float secondsPerBeat = 60.0f / GetCurrentBPM();
        float beatTime = fmod(timer, secondsPerBeat);

        return (beatTime < badWindow || beatTime > secondsPerBeat - badWindow);
    }


    float GetBeatPhase()
    {
		double currentBeatTime = GetBeatTime(); // New Function
        float secondsPerBeat = 60.0f / GetCurrentBPM();
        float beatTime = fmod(currentBeatTime, secondsPerBeat);
        return (float)(beatTime / secondsPerBeat);
    }

    void Update()
    {
        timer = Audio::GetPositionSeconds() + audioOffset;
		UpdateBPM(timer);
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

        int pressedMask = Input::GetPressedMask(); //This requires modifying if you want to add detecting pressing more than 1 key at the same time.

        if (pressedMask != 0 && nextNoteIndex < notes.size())
        {
            Note& note = notes[nextNoteIndex];

            double diff = timer - note.time;

            if (diff >= -badWindow && diff <= badWindow)
            {
                if ((pressedMask & note.keyMask) == note.keyMask)
                {
                    if (fabs(diff) <= perfectWindow) //Perfect
                    {
                        combo++;
                        UpdateMultiplier();
                        score += 300 * multiplier;
                        lastJudgement = Judgement::Perfect;
                        hitPerfect++;
                        totalNotes++;
                        if (combo > maxCombo)
                            maxCombo = combo;
						Audio::PlayOnce("CarbonRhythmAssets/CRsoft.mp3", 0.2f);
                    }
                    else if (fabs(diff) <= goodWindow) //Good
                    {
                        combo++;
                        UpdateMultiplier();
                        score += 150 * multiplier;
                        lastJudgement = Judgement::Good;
                        hitGood++;
                        totalNotes++;
                        if (combo > maxCombo)
                            maxCombo = combo;
						Audio::PlayOnce("CarbonRhythmAssets/CRsoft.mp3", 0.2f);
                    }
                    else //Bad
                    {
                        score += 50;
                        combo = 0;
                        multiplier = 1;
                        lastJudgement = Judgement::Bad;
                        hitBad++;
                        totalNotes++;
						Audio::PlayOnce("CarbonRhythmAssets/CRsoft.mp3", 0.2f);
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
                else //Miss
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

	double GetNoteBeat(size_t index)
	{
		if (index < notes.size())
			return notes[index].beatTime;
		return 0.0;
	}

	double GetBeatTimeAtTime(double seconds)
	{
		if (bpmChanges.empty())
			return seconds * (bpm / 60.0);

		double beatTime = 0.0;
		double lastTime = 0.0;
		double lastBPM = bpm; // Starting BPM (from file)

		for (size_t i = 0; i < bpmChanges.size(); ++i)
		{
			const BPMChange& change = bpmChanges[i];
			if (seconds >= change.time)
			{
				double deltaSec = change.time - lastTime;
				beatTime += deltaSec * (lastBPM / 60.0);
				lastTime = change.time;
				lastBPM = change.bpm;
			}
			else
				break;
		}

		// Last section (from last change to requested time)
		if (seconds > lastTime)
		{
			double deltaSec = seconds - lastTime;
			beatTime += deltaSec * (lastBPM / 60.0);
		}

		return beatTime;
	}

    void AddNote(double time, int mask)
    {
        notes.push_back({ time, GetBeatTimeAtTime(time), mask, false, 0.0f });
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

	void SetBPMChanges(const std::vector<BPMChange>& changes)
	{
		bpmChanges = changes;
		currentBpmIndex = 0;
		totalBeatTime = 0.0;
		lastBPMChangeTime = 0.0;

		// If there is a first change at time 0, use its BPM immediately
		if (!bpmChanges.empty() && bpmChanges[0].time == 0.0)
		{
			bpm = bpmChanges[0].bpm;
		}
	}

	double GetCurrentBPM()
	{
		return bpm;
	}

	double GetBeatTime()
	{
		// If no changes, simple formula
		if (bpmChanges.empty())
			return timer * (bpm / 60.0);

		// If we haven't reached first change yet
		if (currentBpmIndex == 0 && timer < bpmChanges[0].time)
			return timer * (bpm / 60.0);

		// Normal case: totalBeatTime is: beats up to lastBPMChangeTime
		double elapsedSinceLastChange = timer - lastBPMChangeTime;
		if (elapsedSinceLastChange < 0.0) elapsedSinceLastChange = 0.0;
		return totalBeatTime + elapsedSinceLastChange * (bpm / 60.0);
	}

	void ToggleAutoplay()  // Auto-play Support
	{
		g_autoplay = !g_autoplay;
		OutputDebugStringA(g_autoplay ? "Autoplay: ON" : "Autoplay: OFF");
	}

	bool IsAutoplayEnabled()
	{
		return g_autoplay;
	}
	double GetTimeUntilNextNote()
	{
		if (nextNoteIndex < notes.size())
			return notes[nextNoteIndex].time - timer;
		return 999999.0;
	}

	int GetNextNoteMask()
	{
		if (nextNoteIndex < notes.size())
			return notes[nextNoteIndex].keyMask;
		return 0;
	}

	double GetBadWindow()
	{
		return badWindow;
	}

	double GetPerfectWindow()
	{
		return perfectWindow;
	}
}
