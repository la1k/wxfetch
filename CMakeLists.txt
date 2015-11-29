cmake_minimum_required(VERSION 2.6)
set(CMAKE_BUILD_TYPE Debug)
project(wxfetch)

add_executable(wxtrack wxtrack/main.cpp)
TARGET_LINK_LIBRARIES(wxtrack predict)

add_executable(receiver receiver/receiver.cpp receiver/main.cpp)
FIND_PACKAGE(Qt4 REQUIRED)
INCLUDE(${QT_USE_FILE})
ADD_DEFINITIONS(${QT_DEFINITIONS})
target_link_libraries(receiver gnuradio-audio boost_system gnuradio-runtime boost_date_time gnuradio-blocks gnuradio-qtgui ${QT_LIBRARIES} gnuradio-filter gnuradio-osmosdr gnuradio-analog)
