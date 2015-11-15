cmake_minimum_required(VERSION 2.6)
set(CMAKE_BUILD_TYPE Debug)
project(wxfetch)

add_executable(wxfetch src/main.cpp)# src/receiver.cpp)
TARGET_LINK_LIBRARIES(wxfetch predict)# gnuradio-blocks gnuradio-runtime gnuradio-analog gnuradio-filter gnuradio-osmosdr boost_system)



