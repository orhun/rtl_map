# Minimum CMake Version: 3.13
cmake_minimum_required(VERSION 3.13)

# Project rtl_map
project(rtl_map C)

# Add source file
add_executable(rtl_map rtl_map.c)

# Check FFTW3
# http://www.fftw.org/
message(STATUS "Checking FFTW3...")
find_library(FFTW_LIB fftw3)
if(FFTW_LIB)
  message(STATUS "FFTW3 found!")
  TARGET_LINK_LIBRARIES(rtl_map fftw3)
else()
  message(WARNING "Cannot find FFTW3!")
  include(ExternalProject)
  ExternalProject_Add(project_fftw
    URL http://www.fftw.org/fftw-3.3.8.tar.gz
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/fftw
    CONFIGURE_COMMAND ${CMAKE_CURRENT_BINARY_DIR}/fftw/src/project_fftw/configure
     --prefix=${CMAKE_CURRENT_BINARY_DIR}/fftw/install
    INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/fftw/install
  )
  add_library(fftw STATIC IMPORTED)
  set(lib_fftw_name ${CMAKE_STATIC_LIBRARY_PREFIX}fftw3${CMAKE_STATIC_LIBRARY_SUFFIX})
  set_target_properties(fftw PROPERTIES IMPORTED_LOCATION 
    ${CMAKE_CURRENT_BINARY_DIR}/fftw/install/lib/${lib_fftw_name})
  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/fftw/install/include)
  add_dependencies(rtl_map project_fftw)
  TARGET_LINK_LIBRARIES(rtl_map fftw)
endif()

# Check rtl_sdr
# https://osmocom.org/projects/rtl-sdr/wiki/Rtl-sdr
message(STATUS "Checking RTL_SDR...")
find_library(RTLSDR_LIB rtlsdr)
if(RTLSDR_LIB)
  message(STATUS "RTL_SDR found!")
  TARGET_LINK_LIBRARIES(rtl_map rtlsdr)
else()
  message(WARNING "Cannot find RTL_SDR!")
  include(ExternalProject)
  ExternalProject_Add(rtl_sdr_project
    URL https://github.com/steve-m/librtlsdr/archive/0.6.0.tar.gz
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/rtl_sdr
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/rtl_sdr/install
  )
  add_library(rtl_sdr STATIC IMPORTED)
  # Link libusb to rtl_sdr
  TARGET_LINK_LIBRARIES(rtl_sdr INTERFACE usb-1.0)
  set(rtl_sdr_lib ${CMAKE_STATIC_LIBRARY_PREFIX}rtlsdr${CMAKE_STATIC_LIBRARY_SUFFIX})
  set_target_properties(rtl_sdr PROPERTIES IMPORTED_LOCATION 
    ${CMAKE_CURRENT_BINARY_DIR}/rtl_sdr/install/lib/${rtl_sdr_lib})
  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/rtl_sdr/install/include)
  add_dependencies(rtl_map rtl_sdr_project)
  TARGET_LINK_LIBRARIES(rtl_map rtl_sdr)
endif()

# Link math library
TARGET_LINK_LIBRARIES(rtl_map m)

# Installation
INSTALL(TARGETS rtl_map RUNTIME DESTINATION bin)
