# Minimum CMake Version: 3.13
cmake_minimum_required(VERSION 3.13)

# Project rtl_map
project(rtl_map C)

# Check FFTW3
# http://www.fftw.org/
message(STATUS "Checking FFTW3...")
find_library(FFTW_LIB fftw3)
if(FFTW_LIB)
  message(STATUS "FFTW3 found!")
else()
  message(FATAL_ERROR "Cannot find FFTW3!")
endif()

# Check rtl_sdr
# https://osmocom.org/projects/rtl-sdr/wiki/Rtl-sdr
message(STATUS "Checking RTL_SDR...")
find_library(RTLSDR_LIB rtlsdr)
if(RTLSDR_LIB)
  message(STATUS "RTL_SDR found!")
else()
  message(FATAL_ERROR "Cannot find RTL_SDR!")
endif()

# Add source file
add_executable(rtl_map rtl_map.c)

# Link fftw3, rtlsdr, math
TARGET_LINK_LIBRARIES(rtl_map fftw3 rtlsdr m)

# Installation
INSTALL(TARGETS rtl_map RUNTIME DESTINATION bin)
