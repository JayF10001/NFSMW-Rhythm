#include <Windows.h>
#include "Input.h"
#include "Rhythm/RhythmSystem.h"

namespace Input
{
    static int rhythmCurrentMask = 0;   // used for rhythm system (downMask)
    static int prevRhythmMask = 0;
    static int downMask = 0;
	
	static int visualCurrentMask = 0;   // used for DanceController (held mask)

    static int gKeyCodes[6] = { 85, 74, 75, 73, 76, 79 };
	
	// Autoplay state
    static float hitTimer = 0.0f;        	// short timer for rhythm system (new press)
    static int hitMask = 0;               	// mask to apply for hit
    static float visualHoldTimer = 0.0f; 	// longer timer for visual feedback (held mask)
    static int visualHoldMask = 0;        	// mask to hold for visual
    static size_t lastProcessedNote = (size_t)-1;

    void Update()
    {
        // 1. IF Autoplay turned off - Read players input. If Turned on, then its blocked.
		if (!Rhythm::IsAutoplayEnabled()) 
		{
			int realMask = 0;
			for (int i = 0; i < 6; i++) // Normal player input
			{
				if (GetAsyncKeyState(gKeyCodes[i]) & 0x8000)
					realMask |= 1 << i;
			}
			rhythmCurrentMask = realMask;
            visualCurrentMask = realMask;
            hitTimer = 0.0f;
            visualHoldTimer = 0.0f;
		}
		else
        {
            // 2. Autoplay: check if we have a valid note in perfect window and not yet hit
            if (Rhythm::HasNextNote())
            {
                size_t currentIdx = Rhythm::GetCurrentNoteIndex();
                double timeToNext = Rhythm::GetTimeUntilNextNote();
                double perfectWin = Rhythm::GetPerfectWindow();

                if (fabs(timeToNext) <= perfectWin && !Rhythm::IsNoteHit(currentIdx) && lastProcessedNote != currentIdx)
                {
                    int noteMask = Rhythm::GetNextNoteMask();
                    if (noteMask != 0)
                    {
                        // Short hit impulse (for rhythm system)
                        hitMask = noteMask;
                        hitTimer = 0.02f;   // 1-2 frames @60fps, enough to generate downMask

                        // Long visual hold (for DanceController)
                        visualHoldMask = noteMask;
                        visualHoldTimer = 0.1f;  // ~6 frames, enough for car to react visually

                        lastProcessedNote = currentIdx;
                    }
                }
            }

            // Rhythm current mask: only when hitTimer active
            if (hitTimer > 0.0f)
                rhythmCurrentMask = hitMask;
            else
                rhythmCurrentMask = 0;

            // Visual current mask: when visualHoldTimer active
            if (visualHoldTimer > 0.0f)
                visualCurrentMask = visualHoldMask;
            else
                visualCurrentMask = 0;
        }

        // 3. Update downMask for rhythm system using rhythmCurrentMask. Here previously also was CurrentMask = realMask.
        downMask = rhythmCurrentMask & (~prevRhythmMask);
        prevRhythmMask = rhythmCurrentMask;
		
		// 4. Decrement Autoplay timer (after calculating downMask) when autoplay=on
        if (Rhythm::IsAutoplayEnabled())
        {
            float dt = Rhythm::GetDeltaTime();
            if (hitTimer > 0.0f)
            {
                hitTimer -= dt;
                if (hitTimer < 0.0f) hitTimer = 0.0f;
            }
            if (visualHoldTimer > 0.0f)
            {
                visualHoldTimer -= dt;
                if (visualHoldTimer < 0.0f) visualHoldTimer = 0.0f;
            }
        }
    }


    int GetPressedMask()
    {
        return downMask;
    }

    int GetHeldMask() //This is needed if you want to add detecting pressing more than 1 key. Also previously returned CurrentMask
    {
        return visualCurrentMask;
    }

    void SetKey(int lane, int vk)
    {
        if (lane >= 0 && lane < 6)
            gKeyCodes[lane] = vk;
    }
}