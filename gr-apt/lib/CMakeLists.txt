# Copyright 2011,2012 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# GNU Radio is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# GNU Radio is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Radio; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.


################################
# Setup QT file generation
################################

set(qtgui_moc_hdrs image_viewer.h)
QT4_WRAP_CPP(qtgui_moc_sources ${qtgui_moc_hdrs})


########################################################################
# Setup library
########################################################################
include(GrPlatform) #define LIB_SUFFIX

include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})

list(APPEND apt_sources
    decode_ff_impl.cc
    image_file_f_impl.cc
    image_widget_f_impl.cc
    image_viewer.cpp
    ${qtgui_moc_sources}
)

set(apt_sources "${apt_sources}" PARENT_SCOPE)
if(NOT apt_sources)
	MESSAGE(STATUS "No C++ sources... skipping lib/")
	return()
endif(NOT apt_sources)

add_library(gnuradio-apt SHARED ${apt_sources})
target_link_libraries(gnuradio-apt ${Boost_LIBRARIES} ${GNURADIO_ALL_LIBRARIES} ${OpenCV_LIBRARIES} ${wxdecode_LIBRARIES} ${QT_LIBRARIES})
set_target_properties(gnuradio-apt PROPERTIES DEFINE_SYMBOL "gnuradio_apt_EXPORTS")

if(APPLE)
    set_target_properties(gnuradio-apt PROPERTIES
        INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib"
    )
endif(APPLE)

########################################################################
# Install built library files
########################################################################
install(TARGETS gnuradio-apt
    LIBRARY DESTINATION lib${LIB_SUFFIX} # .so/.dylib file
    ARCHIVE DESTINATION lib${LIB_SUFFIX} # .lib file
    RUNTIME DESTINATION bin              # .dll file
)

########################################################################
# Build and register unit test
########################################################################
include(GrTest)

include_directories(${CPPUNIT_INCLUDE_DIRS})

list(APPEND test_apt_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/test_apt.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/qa_apt.cc
)

add_executable(test-apt ${test_apt_sources})

target_link_libraries(
  test-apt
  ${GNURADIO_RUNTIME_LIBRARIES}
  ${Boost_LIBRARIES}
  ${CPPUNIT_LIBRARIES}
  gnuradio-apt
)

GR_ADD_TEST(test_apt test-apt)
