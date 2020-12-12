# audio-wake-up
基于alsa实现录音和播放，基于mfcc和DTW实现唤醒和识别，基于mpg123实现mp3解码，best.txt是我的语音“你好”的mfcc。
在检测到“你好”的时候，会播放一段MP3音乐。

#编译方式
安装alsa-lib和mpg123，确保动态链接库和头文件可以被编译器找到，然后cmake .& make & make install.

#运行方式
./audio_test（确保同一文件夹下有best.txt,程序里写的mp3路径可以被找到。）

