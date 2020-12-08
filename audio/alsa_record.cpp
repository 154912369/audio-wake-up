/* 
 * file alsa_record.cpp, belongs to audio_record project
 * CopyrightEric Bachard 2020 August 23th   17:55:00   CEST
 * License : GPL v3
 * See: http://www.gnu.org/licenses/gpl-3.0.html
 */


#include <iostream>
#include <time.h>
#include <fstream>
#include <string>
#include <thread>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <alsa/asoundlib.h>

#include "audio/alsa_record.hpp"
#include "audio/getch.h"

// this is the bitrate
//#define MAX_BUF_SIZE	512
#define MAX_BUF_SIZE	1024
//#define MAX_BUF_SIZE	4096
//  8s
//#define MAX_SAMPLES	256000
#define MAX_SAMPLES	512000


AlsaRecord::AlsaRecord()
{
};


AlsaRecord::~AlsaRecord()
{
};

void AlsaRecord::init()
{   
    

     int numCepstra = 12;
    int numFilters = 40;
    int samplingRate =16000;
    int winLength = 25;
    int frameShift =10;
    int lowFreq = 50;
    int highFreq =samplingRate/2;


    srate=samplingRate;
    init_soundcard();
    std::cout << "Open and init done" << "\n";
    // Initialise MFCC class instance
    this->mfcc= new MFCC(samplingRate, numCepstra, winLength, frameShift, numFilters, lowFreq, highFreq);
    int dtw_length=120;
    double* compare_daa=new double[dtw_length*(numCepstra+1)];
    std::ifstream in("test.txt");  
    std::string s;
    int i=0;
    while (getline(in, s)) {//读取每一行
        compare_daa[i]=atof(s.c_str());
        std::cout<<compare_daa[i]<<std::endl;
        i++;
        if(i>=dtw_length*13){
            break;
        }
         
    }

    this->ad= new AudioData(dtw_length,numCepstra+1,compare_daa,dtw_length);


}


/// Open and init default sound card params
int AlsaRecord::init_soundcard()
{
    int err = 0;
    
    if ((err = snd_pcm_open(&capture_handle, snd_device, SND_PCM_STREAM_CAPTURE, 0)) < 0)

    {
        std::cerr << "cannot open audio device " << snd_device << "(" << snd_strerror(err) << ", " << err << ")" << "\n";
        return OPEN_ERROR;
    }

    if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0)
    {
        std::cerr << "cannot allocate hardware parameter structure " << "(" << snd_strerror(err) << ", " << err << ")" << "\n";
        return MALLOC_ERROR;
    }

    if ((err = snd_pcm_hw_params_any(capture_handle, hw_params)) < 0)
    {
        std::cerr << "cannot initialize hardware parameter structure " << "(" << snd_strerror(err) << ", " << err << ")" << "\n";
        return ANY_ERROR;
    }

    if ((err = snd_pcm_hw_params_set_access(capture_handle, hw_params,
                SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
    {
        std::cerr << "cannot set access type " << "(" << snd_strerror(err) << ", " << err << ")" << "\n";
        return ACCESS_ERROR;
    }

    if ((err = snd_pcm_hw_params_set_format(capture_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0)
    {
        std::cerr << "cannot set sample format " << "(" << snd_strerror(err) << ", " << err << ")" << "\n";
        return FORMAT_ERROR;
    }

    if ((err = snd_pcm_hw_params_set_rate_near(capture_handle, hw_params,
                &srate, 0)) < 0)
    {
        std::cerr << "cannot set sample rate " << "(" << snd_strerror(err) << ", " << err << ")" << "\n";
        return RATE_ERROR;
    }

    if ((err = snd_pcm_hw_params_set_channels(capture_handle, hw_params, nchan))< 0)
    {
        std::cerr << "cannot set channel count " << "(" << snd_strerror(err) << ", " << err << ")" << "\n";
        return CHANNELS_ERROR;
    }

    if ((err = snd_pcm_hw_params(capture_handle, hw_params)) < 0)
    {
        std::cerr << "cannot set parameters " << "(" << snd_strerror(err) << ", " << err << ")" << "\n";
        return PARAMS_ERROR;
    }

    if ((err = snd_pcm_prepare(capture_handle)) < 0)
    {
        std::cerr << "cannot prepare audio interface for use " << "(" << snd_strerror(err) << ", " << err << ")" << "\n";
        return PREPARE_ERROR;
    }

    if ((err = snd_pcm_start(capture_handle)) < 0)
    {
        std::cerr << "cannot start soundcard " << "(" << snd_strerror(err) << ", " << err << ")" << "\n";
        return START_ERROR;
    }

    return EXIT_SUCCESS;
}

int AlsaRecord::close_soundcard()
{
    return snd_pcm_close(capture_handle);
}



int AlsaRecord::do_record()
{
    this->b_quit=true;
    int err = 0;
    fwav = fopen("test.wav", "wb");
    wav_h.ChunkID[0]     = 'R';
    wav_h.ChunkID[1]     = 'I';
    wav_h.ChunkID[2]     = 'F';
    wav_h.ChunkID[3]     = 'F';

    wav_h.Format[0]      = 'W';
    wav_h.Format[1]      = 'A';
    wav_h.Format[2]      = 'V';
    wav_h.Format[3]      = 'E';

    wav_h.Subchunk1ID[0] = 'f';
    wav_h.Subchunk1ID[1] = 'm';
    wav_h.Subchunk1ID[2] = 't';
    wav_h.Subchunk1ID[3] = ' ';

    wav_h.Subchunk2ID[0] = 'd';
    wav_h.Subchunk2ID[1] = 'a';
    wav_h.Subchunk2ID[2] = 't';
    wav_h.Subchunk2ID[3] = 'a';

    wav_h.NumChannels = 2;
    wav_h.BitsPerSample = 16;
    wav_h.Subchunk2Size = 300 * MAX_SAMPLES * (uint32_t) wav_h.NumChannels * (uint32_t) wav_h.BitsPerSample / 8;
    //wav_h.Subchunk2Size = 0xFFFFFFFF;
    wav_h.ChunkSize = (uint32_t) wav_h.Subchunk2Size + 36;
    wav_h.Subchunk1Size = 16;
    wav_h.AudioFormat = 1;
    wav_h.SampleRate = srate;
    wav_h.ByteRate = (uint32_t) wav_h.SampleRate
                     * (uint32_t) wav_h.NumChannels
                     * (uint32_t) wav_h.BitsPerSample / 8;
    wav_h.BlockAlign = (uint32_t) wav_h.NumChannels * (uint32_t) wav_h.BitsPerSample / 8;

    fwrite(&wav_h, 1, sizeof(wav_h), fwav);
    int16_t wav_data[(mfcc->winLengthSamples-mfcc->frameShiftSamples) *2];
     std::cout<<"start record, first is "<<(mfcc->winLengthSamples-mfcc->frameShiftSamples) *2<<"second is:"<<mfcc->frameShiftSamples *2<<std::endl;

   if ((err = snd_pcm_readi(capture_handle, wav_data, mfcc->winLengthSamples-mfcc->frameShiftSamples)) != mfcc->winLengthSamples-mfcc->frameShiftSamples)
        {
            std::cerr << "read from audio interface failed " << "(" << snd_strerror(err) << ", " << err << ")" << "\n";

            if (err == -32) // Broken pipe
            {
                if ((err = snd_pcm_prepare(capture_handle)) < 0)
                {
                    std::cerr << "cannot prepare audio interface for use " << "(" << snd_strerror(err) << ", " << err << ")" << "\n";
                    return PREPARE_ERROR;
                }
            }
            else
                return SNDREAD_ERROR;

        }


            //std::cout<<"data is"<<std::endl;
            //std::cout.flush();
            //std::cout<<"data is"<<wav_data[0]<<wav_data[MAX_BUF_SIZE * 4 -1]<<std::endl;
            mfcc->addPreData(wav_data);
             fwrite(wav_data, 1, (mfcc->winLengthSamples-mfcc->frameShiftSamples)*4, fwav);
 int16_t new_wav_data[mfcc->frameShiftSamples *2];
 double distance;
    v_d result;
    unsigned long int ncount=0;
    clock_t start,ends;
    start=clock();
    do
    {
        if ((err = snd_pcm_readi(capture_handle, new_wav_data, mfcc->frameShiftSamples)) != mfcc->frameShiftSamples)
        {
            std::cerr << "read from audio interface failed " << "(" << snd_strerror(err) << ", " << err << ")" << "\n";

            if (err == -32) // Broken pipe
            {
                if ((err = snd_pcm_prepare(capture_handle)) < 0)
                {
                    std::cerr << "cannot prepare audio interface for use " << "(" << snd_strerror(err) << ", " << err << ")" << "\n";
                    return PREPARE_ERROR;
                }
                else{
                    ends=clock();
                    std::cerr<<"time is"<<ends-start<<std::endl;
                }
            }
            else
                return SNDREAD_ERROR;

        }
   
       
    fwrite(new_wav_data, 1, (mfcc->frameShiftSamples)*4, fwav);
    

         ad->put_number(mfcc->processFrame(new_wav_data,mfcc->frameShiftSamples, true));
    distance=ad->dtw();
        if(distance<2100){std::cout<<"distance less than 1600,:"<<distance<<std::endl;}
        
   

  

                ncount++;
    } while (b_quit); /*esc */
    fclose(fwav);
    ad->get_data();
    std::ofstream ofile;
    ofile.open("test.txt",std::ios::out);
    for(int i=0;i<ad->dim*ad->data_length;i++){
        ofile<<ad->right_data[i]<<"\n";

    }

    ofile.close();

std::cout<<"ncount is "<<ncount<<std::endl;
    return EXIT_SUCCESS;
}




 void keyboard_loop(AlsaRecord* ar)
{
    char c = 0;
    while (c != 27) // esc
    {
        c = 0;
        c=getch();
    }
    if (c == 27) // esc
        ar->b_quit = false;
}