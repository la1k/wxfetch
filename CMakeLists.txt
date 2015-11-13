cmake_minimum_required(VERSION 2.8)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -std=gnu99")

add_library(wxdecode src/dsp.cpp src/filter.cpp)
add_executable(wxdecode-bin src/main.cpp)
target_link_libraries(wxdecode-bin m sndfile wxdecode opencv_core opencv_highgui)
