#pragma once
#include <vector>

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
	
	struct BPMChange { 	// New structure representing Point, where changes in BPM will be read
		double time;  	// Time in seconds FROM Song Start
		double bpm;   	// New BPM value that will be used from that time
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
    int GetMultiplier();
    static void UpdateMultiplier();
    double GetAccuracy();
    float GetDeltaTime();
    int GetLastHitMask();
    int GetLastHitNoteIndex();
    bool IsNoteHit(size_t index);
    float GetNoteHitVisualTimer(size_t index);
	void SetBPMChanges(const std::vector<BPMChange>& changes);
	double GetCurrentBPM();
	double GetNoteBeat(size_t index);
	double GetBeatTime();  							// Returns time in "beats", taking into account BPM changes
	double GetBeatTimeAtTime(double seconds);
	void ToggleAutoplay(); 							// Here are values for Auto-play Support
	bool IsAutoplayEnabled();
	double GetTimeUntilNextNote();  				// time to next note (negative if after time)
	int GetNextNoteMask();          				// nearest note mask
	double GetBadWindow();          				// needed in Input.cpp
	double GetPerfectWindow();
}