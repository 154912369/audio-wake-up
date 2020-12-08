cmake_minimum_required (VERSION 3.10)
project (audio)
add_compile_options(-g)
include_directories(/home/renweijie/code/cpp/Linux_Alsa_Audio_Record/)
aux_source_directory(./audio DIR_SRCS)
aux_source_directory(./data DATA_SRCS)
aux_source_directory(./mfcc MFCC_SRCS)
MESSAGE( STATUS "this var key = ${DIR_SRCS}.")


set(SOURCES_FILES main.cpp)

add_executable(trans  trans2mfcc.cpp ${DIR_SRCS} ${DATA_SRCS} ${MFCC_SRCS})
target_link_libraries(trans -lasound -lpthread )
# add_executable(audio_test  ${DIR_SRCS} ${DATA_SRCS} ${MFCC_SRCS} ${SOURCES_FILES})
# target_link_libraries(audio_test -lasound -lpthread )
