#include "AudioSystem.h"
#include <bass.h>

namespace Audio
{
    static HSTREAM gStream = 0;

    bool Initialize()
    {
        static bool initialized = false;
        if (initialized)
            return true;

        if (BASS_Init(-1, 44100, 0, 0, NULL))
        {
            initialized = true;
            return true;
        }

        return false;
    }


    bool Load(const std::string& path)
    {
        Initialize();
        if (gStream)
        {
            BASS_StreamFree(gStream);
            gStream = 0;
        }

        gStream = BASS_StreamCreateFile(
            FALSE,
            path.c_str(),
            0,
            0,
            BASS_STREAM_PRESCAN
        );

        return gStream != 0;
    }

    void Play()
    {
        if (gStream)
            BASS_ChannelPlay(gStream, FALSE);
    }

    void Stop()
    {
        if (gStream)
            BASS_ChannelStop(gStream);
    }

    double GetPositionSeconds()
    {
        if (!gStream)
            return 0.0;

        QWORD pos = BASS_ChannelGetPosition(gStream, BASS_POS_BYTE);
        return BASS_ChannelBytes2Seconds(gStream, pos);
    }
    bool IsFinished()
    {
        if (!gStream) return true;
        return BASS_ChannelIsActive(gStream) == BASS_ACTIVE_STOPPED;
    }

	void Audio::PlayOnce(const std::string& path, float volume) //Code needed to play Hit-Sounds
	{
		HSTREAM stream = BASS_StreamCreateFile(FALSE, path.c_str(), 0, 0, BASS_STREAM_PRESCAN);
		if (stream)
		{
			BASS_ChannelSetAttribute(stream, BASS_ATTRIB_VOL, volume);

			// Sync after playback ends to free the stream. Otherwise this might cause memory leak.
			BASS_ChannelSetSync(stream, BASS_SYNC_END, 0,
				[](HSYNC, DWORD, DWORD, void* user) {
					HSTREAM s = reinterpret_cast<HSTREAM>(user);
					BASS_StreamFree(s);
				},
				reinterpret_cast<void*>(stream));   // <-- key change: casting to void*

			BASS_ChannelPlay(stream, FALSE);
		}
	}
}