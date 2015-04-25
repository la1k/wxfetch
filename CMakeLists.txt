cmake_minimum_required(VERSION 2.6)
set(CMAKE_BUILD_TYPE Debug)
project(wxtrack)

add_executable(wxtrack main.cpp tracker.cpp)
TARGET_LINK_LIBRARIES(wxtrack predict gnuradio-blocks gnuradio-runtime gnuradio-analog gnuradio-filter gnuradio-osmosdr boost_system)



