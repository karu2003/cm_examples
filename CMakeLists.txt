cmake_minimum_required(VERSION 3.13)

# Toolchain must be set before project() call.
if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_LIST_DIR}/coralmicro/cmake/toolchain-arm-none-eabi-gcc.cmake)
endif()

project(coralmicro-app)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)

include_directories(coralmicro)
add_subdirectory(coralmicro)

include_directories(lib/base)
add_subdirectory(lib/base)

add_subdirectory(audio_scoppy)
add_subdirectory(dac_out)
add_subdirectory(mic_kissfft)
add_subdirectory(simple_fft_test)
add_subdirectory(timer_interrupt)
add_subdirectory(generator)
# add_subdirectory(cwt)
add_subdirectory(arm_fft)
add_subdirectory(pffft)
add_subdirectory(convolution)
add_subdirectory(xCount)
add_subdirectory(arm_fCWT)
add_subdirectory(arm_cfft)
add_subdirectory(conv_cwt)
add_subdirectory(correlate)


