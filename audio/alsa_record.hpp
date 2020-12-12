/* 
 * file alsa_record.cpp, belongs to audio_record project
 * Copyright Eric Bachard 2020 August 23th   17:55:00   CEST
 * License : GPL v3
 * See: http://www.gnu.org/licenses/gpl-3.0.html
 */
#include <stdint.h>
#include <alsa/asoundlib.h>
#include "mfcc/mfcc.cpp"
#include "tasks/task.cpp"
#ifndef __ALSA_RECORD_HPP__
#define __ALSA_RECORD_HPP__

#define OPEN_ERROR 1
#define MALLOC_ERROR 2
#define ANY_ERROR 3
#define ACCESS_ERROR 4
#define FORMAT_ERROR 5
#define RATE_ERROR 6
#define CHANNELS_ERROR 7
#define PARAMS_ERROR 8
#define PREPARE_ERROR 9
#define FOPEN_ERROR 10
#define FCLOSE_ERROR 11
#define SNDREAD_ERROR 12
#define START_ERROR 13

struct wav_header // Wav file header structure
{
    uint8_t ChunkID[4];     //riff
    uint32_t ChunkSize;     //Subchunk2Size + 36;
    uint8_t Format[4];      //wave
    uint8_t Subchunk1ID[4]; //fmt
    uint32_t Subchunk1Size;
    uint16_t AudioFormat;
    uint16_t NumChannels;
    uint32_t SampleRate;
    uint32_t ByteRate;
    uint16_t BlockAlign;
    uint16_t BitsPerSample;
    uint8_t Subchunk2ID[4]; //data
    uint32_t Subchunk2Size; //  MAX_BUF_SIZE * ncount * (uint32_t) wav_h.NumChannels * (uint32_t) wav_h.BitsPerSample / 8;
};

class AlsaRecord
{
public:
    // Ctor()
    AlsaRecord();

    // Dtor()
    ~AlsaRecord();

    void init(void);

    struct wav_header wav_h;

    snd_pcm_t *capture_handle;
    snd_pcm_hw_params_t *hw_params;
    snd_pcm_info_t *s_info;
    MFCC *mfcc;
    AudioData *ad;
    task tk=task();
    unsigned int srate = 16000;
    unsigned int nchan = 2;
    bool b_quit;

    char *wav_name;

    int init_soundcard(void);
    int close_soundcard(void);
    int init_wav_header(void);
    int init_wav_file(char *);
    int close_wav_file(void);

    void set_sound_device(const char *aDeviceName) { snd_device = aDeviceName; };
    int do_record();

    inline const char *getSoundDevice(void) { return snd_device; }

private:
    const char *snd_device;
};

#endif /* __ALSA_RECORD_HPP__ */

static bool b_quit = false;
void keyboard_loop(AlsaRecord *);