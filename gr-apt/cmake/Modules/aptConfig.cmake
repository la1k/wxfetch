INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_APT apt)

FIND_PATH(
    APT_INCLUDE_DIRS
    NAMES apt/api.h
    HINTS $ENV{APT_DIR}/include
        ${PC_APT_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    APT_LIBRARIES
    NAMES gnuradio-apt
    HINTS $ENV{APT_DIR}/lib
        ${PC_APT_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(APT DEFAULT_MSG APT_LIBRARIES APT_INCLUDE_DIRS)
MARK_AS_ADVANCED(APT_LIBRARIES APT_INCLUDE_DIRS)

