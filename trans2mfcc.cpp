#include "mfcc/mfcc.cpp"
#include <iostream>
#include <fstream>
struct wav_header // Wav file header structure
{
    uint8_t ChunkID[4];//riff
    uint32_t ChunkSize;//Subchunk2Size + 36;
    uint8_t Format[4];//wave
    uint8_t Subchunk1ID[4];//fmt
    uint32_t Subchunk1Size;
    uint16_t AudioFormat;
    uint16_t NumChannels;
    uint32_t SampleRate;
    uint32_t ByteRate;
    uint16_t BlockAlign;
    uint16_t BitsPerSample;
    uint8_t Subchunk2ID[4];//data
    uint32_t Subchunk2Size;//  MAX_BUF_SIZE * ncount * (uint32_t) wav_h.NumChannels * (uint32_t) wav_h.BitsPerSample / 8;
};
int main(){
     int numCepstra = 12;
    int numFilters = 40;
    int samplingRate =16000;
    int winLength = 25;
    int frameShift =10;
    int lowFreq = 50;
    int highFreq =samplingRate/2;

    FILE *fwav = fopen("test2.wav", "wb");
    wav_header wav_h;
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
    wav_h.Subchunk2Size = 300 * 512000 * (uint32_t) wav_h.NumChannels * (uint32_t) wav_h.BitsPerSample / 8;
    //wav_h.Subchunk2Size = 0xFFFFFFFF;
    wav_h.ChunkSize = (uint32_t) wav_h.Subchunk2Size + 36;
    wav_h.Subchunk1Size = 16;
    wav_h.AudioFormat = 1;
    wav_h.SampleRate =16000;
    wav_h.ByteRate = (uint32_t) wav_h.SampleRate
                     * (uint32_t) wav_h.NumChannels
                     * (uint32_t) wav_h.BitsPerSample / 8;
    wav_h.BlockAlign = (uint32_t) wav_h.NumChannels * (uint32_t) wav_h.BitsPerSample / 8;
    fwrite(&wav_h, 1, sizeof(wav_h), fwav);
    MFCC mfcc(samplingRate, numCepstra, winLength, frameShift, numFilters, lowFreq, highFreq);
    std::ifstream file("/home/renweijie/code/cpp/Linux_Alsa_Audio_Record/build/test4.wav", std::ios::binary);
    int16_t wav_data[(mfcc.winLengthSamples-mfcc.frameShiftSamples) *2];
    file.read((char *)wav_data, 44);
    file.read((char *)wav_data, (mfcc.winLengthSamples-mfcc.frameShiftSamples) *4);
    std::cout<<wav_data[0]<<wav_data[1]<<std::endl;
    mfcc.addPreData(wav_data);
    int count=0;
    std::ofstream ofile;
    ofile.open("test.txt",std::ios::out);
   
      
int dtw_length=120;
    double* compare_daa=new double[dtw_length*(numCepstra+1)];
    std::ifstream in("/home/renweijie/code/cpp/Linux_Alsa_Audio_Record/build/best.txt");  
    std::string s;
    int i=0;
     if ( ! in )
      {   
          std::cout << " Error opening "<<std::endl;   
          exit( - 1 );  
      }
    while (getline(in, s)) {//读取每一行
        compare_daa[i]=atof(s.c_str());
        std::cout<<compare_daa[i]<<std::endl;
        i++;
        if(i>=dtw_length*13){
            break;
        }
         
    }

    AudioData ad(dtw_length,numCepstra+1,compare_daa,dtw_length);

   float distance;
    while( file.read((char *)wav_data, (mfcc.frameShiftSamples) *4)){

            fwrite(wav_data, 1, (mfcc.frameShiftSamples)*4, fwav);
           auto  result=mfcc.processFrame(wav_data,mfcc.frameShiftSamples, true);
           ad.put_number(result);
           distance=ad.dtw();
           std::cout<<"distance is"<<distance<<std::endl;
           if(count>0&&count<121){
           for(int i=0;i<result.size();i++){
               ofile<<result[i]<<"\n";
           }
           }

              
       
    
    count +=1;
    }
     ofile.close();
    std::cout<<"count is"<<count<<std::endl;
    fclose(fwav);
    ad.get_data();
    for(int i=0;i<5;i++){
std::cout<<ad.right_data[i]<<std::endl;
    }
       

    ofile.open("test2.txt",std::ios::out);
     for(int i=0;i<13*120;i++){
ofile<<ad.right_data[i]<<"\n";
    }
    ofile.close();
    
    ofile.open("test3.txt",std::ios::out);
     for(int i=0;i<13*120;i++){
ofile<<ad.compare_data[i]<<"\n";
    }
    ofile.close();
    return 0;
}