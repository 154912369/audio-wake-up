#include "audio/alsa_record.hpp"
#include <iostream>
#include<thread>
int main(int argc, char *argv[])
{

        AlsaRecord ar;

        ar.set_sound_device("plughw:PCH,0");
        ar.init();



        std::thread first(keyboard_loop, &ar);

        int anErr = ar.do_record();
        if (anErr !=  EXIT_SUCCESS)
            std::cout << "Pb with do_record() "  << "\n";

        first.join();


        std::cout << "Close default soundcard" << "\n";
        ar.close_soundcard();
    return 0;
}
