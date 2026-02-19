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
}
