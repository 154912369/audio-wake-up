#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <sys/mman.h>
#include <stdint.h>
#include <endian.h>
#include <mpg123.h>
#include <unistd.h>

/*
 based on mpglib (and libmpg123 sample program), code might have been revisited/rewritten, not sure.
 
 mpglib is LGPL, while it has one single output driver that is GPL (the ALSA one) this program doesn't
 use it, instead it deals directly with the ALSA layer following the ALSA guideline/example.
*/

class player
{
  public:
  player(){};
  int i, err, retval;
  snd_pcm_t *playback_handle;
  snd_pcm_hw_params_t *hw_params;
  long rate = 0;
  unsigned int unrate = 0;
  size_t lretval;
  int channels = 0;
  const char *alsa_dev= "hw:0,0";

  mpg123_handle *m;
  char wavbuffer[32768];
  void play(const char *music_file,bool * onging)
  {

    if ((err = snd_pcm_open(&playback_handle, alsa_dev, SND_PCM_STREAM_PLAYBACK, 0)) < 0)
    {
      fprintf(stderr, "Could not open audio device %s (%s)\n", alsa_dev, snd_strerror(err));

     return;
    }

    if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0)
    {
      fprintf(stderr, "Could not setup hardware (%s)\n", snd_strerror(err));
      snd_pcm_close(playback_handle);
      return;
    }

    // init mpg123, decode first chunk and set audio output according to that.
    mpg123_init();
    m = mpg123_new(NULL, &retval);
    if (!m)
    {
      fprintf(stderr, "Unable to init libmpg123: %s\n", mpg123_plain_strerror(retval));
      snd_pcm_close(playback_handle);
      return;
    }

    #ifdef DEBUG
        mpg123_param(m, MPG123_VERBOSE, 2, 0); // leave it a little vebose, just because mpg123's sample told us so
    #endif

    if (mpg123_open(m, music_file) != MPG123_OK)
    {
      fprintf(stderr, "Unable to open file %s!\n", music_file);
      snd_pcm_close(playback_handle);
      return;
    }

    retval = mpg123_read(m, wavbuffer, sizeof(wavbuffer), &lretval);

    if (retval == MPG123_NEW_FORMAT)
    {
      if (mpg123_getformat(m, &rate, &channels, &retval) != MPG123_OK)
      {
        fprintf(stderr, "Error trying to parse stream!\n");
        snd_pcm_close(playback_handle);
        return;
      }
    }
    else
    {
      fprintf(stderr, "No new format rules? (%s) Ayeeee!\n", mpg123_plain_strerror(retval));
      snd_pcm_close(playback_handle);
      return;
    }
    unrate = rate;
    if ((err = snd_pcm_hw_params_any(playback_handle, hw_params)) < 0)
    {
      fprintf(stderr, "Could not initialize hardware parameter structure (%s)\n", snd_strerror(err));
      snd_pcm_close(playback_handle);
      return;
    }
    if ((err = snd_pcm_hw_params_set_access(playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
    {
      fprintf(stderr, "Could not set access type (%s)\n", snd_strerror(err));
      snd_pcm_close(playback_handle);
      return;
    }
    if ((err = snd_pcm_hw_params_set_format(playback_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0)
    {
      fprintf(stderr, "Cannot set sample format (%s)\n", snd_strerror(err));
      snd_pcm_close(playback_handle);
      return;
    }
    if ((err = snd_pcm_hw_params_set_rate_near(playback_handle, hw_params, &unrate, 0)) < 0)
    {
      fprintf(stderr, "Cannot set sample rate to %d (%s)\n", unrate, snd_strerror(err));
      snd_pcm_close(playback_handle);
      return;
    }
    if ((err = snd_pcm_hw_params_set_channels(playback_handle, hw_params, channels)) < 0)
    {
      fprintf(stderr, "Cannot set channel count to %d (%s)\n", channels, snd_strerror(err));
      snd_pcm_close(playback_handle);
      return;
    }
    if ((err = snd_pcm_hw_params(playback_handle, hw_params)) < 0)
    {
      fprintf(stderr, "Cannot set parameters (%s)\n", snd_strerror(err));
      snd_pcm_close(playback_handle);
      return;
    }
    //  snd_pcm_hw_params_free (hw_params);

    if ((err = snd_pcm_prepare(playback_handle)) < 0)
    {
      fprintf(stderr, "Could not prepare audio interface for use (%s)\n", snd_strerror(err));
      snd_pcm_close(playback_handle);
      return;
    }

    while (*onging)
    {
      retval = mpg123_read(m, wavbuffer, sizeof(wavbuffer), &lretval);

      if (retval == MPG123_NEW_FORMAT)
      {
        snd_pcm_drain(playback_handle); // let it play up until here in the former frequency
        if (mpg123_getformat(m, &rate, &channels, &retval) != MPG123_OK)
        {
          fprintf(stderr, "Error while changing stream bitrate/audio format.\n");
          snd_pcm_close(playback_handle);
          return;
        }
        unrate = rate;
        if ((err = snd_pcm_hw_params_set_rate_near(playback_handle, hw_params, &unrate, 0)) < 0)
        {
          fprintf(stderr, "Could not set sample rate to %d (%s)\n", unrate, snd_strerror(err));
          snd_pcm_close(playback_handle);
          return;
        }
        if ((err = snd_pcm_hw_params_set_channels(playback_handle, hw_params, channels)) < 0)
        {
          fprintf(stderr, "Could not set channel count to %d (%s)\n", channels, snd_strerror(err));
          snd_pcm_close(playback_handle);
          return;
        }
        if ((err = snd_pcm_hw_params(playback_handle, hw_params)) < 0)
        {
          fprintf(stderr, "Cannot set parameters (%s)\n", snd_strerror(err));
          snd_pcm_close(playback_handle);
          return;
        }
      }
      else if (retval == MPG123_OK)
      {
        if (lretval)
          snd_pcm_writei(playback_handle, wavbuffer, (lretval / (channels << 1))); // safe to assume 16 bit output always on, so just multiply the number of channels by 2 to get the sample count
        else
          {
            snd_pcm_close(playback_handle);
          return;}
      }
      else if (retval == MPG123_DONE)
      {
        snd_pcm_close(playback_handle);
        return;
      }
      else
      {
        fprintf(stderr, "No new format rules? (%s) Ayeeee!\n", mpg123_plain_strerror(retval));
        snd_pcm_close(playback_handle);
        return;
      }
    }

    // flush alsa and off we go!
    snd_pcm_drain(playback_handle);
    snd_pcm_close(playback_handle);
    snd_pcm_hw_params_free(hw_params);

  }
};